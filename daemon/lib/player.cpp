#include "player.h"

#include <pthread.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "common.h"
#include "decode/decoder.h"
#include "log.hpp"
#include "render/renderer.h"
#include "resource/wallpaper.h"

bool TaskList::empty() {
  std::lock_guard<std::mutex> lock(mtx);
  return cmdQueue.empty();
}

void TaskList::push(std::string action, std::string target) {
  std::lock_guard<std::mutex> lock(mtx);
  cmdQueue.push({action, target});
}

Task TaskList::get() {
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait(lock, [this] { return !cmdQueue.empty(); });
  auto res = cmdQueue.front();
  cmdQueue.pop();
  return res;
}

bool FrameBuffer::empty() {
  std::lock_guard<std::mutex> lock(mtx);
  return bufferQueue.empty();
}

void FrameBuffer::clear() {
  std::lock_guard<std::mutex> lock(mtx);
  std::queue<oneFrame> empty;
  std::swap(bufferQueue, empty);
}

int FrameBuffer::size() {
  std::lock_guard<std::mutex> lock(mtx);
  return bufferQueue.size();
}

void FrameBuffer::push(oneFrame frame) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    bufferQueue.push(std::move(frame));
  }
  cv.notify_one();
}

oneFrame FrameBuffer::get() {
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait(lock, [this] { return !bufferQueue.empty(); });

  oneFrame frame = std::move(bufferQueue.front());
  bufferQueue.pop();
  return frame;
}

void Player::handleTask() {
  while (!tasklist.empty()) {
    Task task = tasklist.get();
    wpList& list = wplists_play[context->currentList];
    int& count = context->currentWallPaper;

    switch (task.action) {
      case Next:
        next();
        break;
      case Previous:
        count = (count > 0 ? (count - 1) : list.size() - 1);
        break;
      case Manual:
        context->loop = !context->loop;
        break;
      case Refresh:
        wplists_play = wplists_resource;
        if (wplists_play[context->currentList].size() == 0) {
          context->currentList = "";
        }
        break;
      case List:
        context->currentList = task.target;
        break;
      case Test:
        context->testPaper = task.target;
        break;
      case Quit:
        context->currentList = "";
        break;
    }

    context->frameBuffer.clear();
  }
}

void Player::next() {
  wpList& list = wplists_play[context->currentList];
  int& count = context->currentWallPaper;

  count = (count < list.size() - 1 ? (count + 1) : 0);
}

void Player::playWallPaper(WallPaper* wp) {
  LOG(INFO) << "Play WallPaper: " << wp->name;

  if (!decoder->prepareDecoder(wp)) {
    next();
    LOG(ERROR) << "Prepare Decoder for " << wp->name << " failed.";
    return;
  }
  decoder->setWallPaperInfo();

  if (wp->type == STATIC) {
    renderer->prepareRender(wp->info.width, wp->info.height, false);

    if (!decoder->decode()) {
      next();
      LOG(ERROR) << "Decode " << wp->name << " failed.";
      return;
    }

    renderer->render(decoder->decodeFinished);
    context->frameBuffer.clear();

    return;
  }

  renderer->prepareRender(wp->info.width, wp->info.height, true,
                          wp->info.frameDelay);

  std::thread decode([&] {
    setThreadName("decode");
    if (!decoder->decode()) {
      next();
      LOG(ERROR) << "Decode " << wp->name << " failed.";
      return;
    }
  });

  renderer->render(decoder->decodeFinished);
  decode.join();
  context->frameBuffer.clear();
}

void Player::testWallPaper() {
  WallPaper paper;
  paper.name = "test";
  paper.path = context->testPaper;
  WallPaper::isWallPaper(context->testPaper);

  switch (WallPaper::isWallPaper(paper.path)) {
    case NONE:
      return;
    case STATIC:
      paper.type = STATIC;
      break;
    case DYNAMIC:
      paper.type = DYNAMIC;
      break;
  }

  auto playWallPaperOnce = [&] {
    std::thread timer([&] {
      setThreadName("timer");
      std::unique_lock<std::mutex> lock(context->mtx);

      // test time
      auto time = 10;

      context->cv.wait_for(lock, std::chrono::seconds(time),
                           [&] { return context->signal.load(); });

      // when the timer end, play must end now.
      if (paper.type == DYNAMIC) {
        context->signal = true;
      }
    });

    do {
      playWallPaper(&paper);
    } while (paper.type == DYNAMIC && !context->signal);

    timer.join();
  };

  playWallPaperOnce();

  if (!tasklist.empty()) {
    handleTask();
  }
  context->signal = false;
  context->testPaper = "";
}

void Player::playList(wpList& list) {
  LOG(INFO) << "Play List: " << context->currentList;

  // when play a new list, loop will be reset
  context->loop = list.time == -1 ? true : false;
  int& count = context->currentWallPaper;

  // when play a new list, count will be reset
  count = 0;

  auto playWallPaperOnce = [&] {
    std::thread timer([&] {
      setThreadName("timer");
      std::unique_lock<std::mutex> lock(context->mtx);

      auto time = list.time;
      if (list.time == -1) {
        time = 60;
      }

      context->cv.wait_for(lock, std::chrono::seconds(time),
                           [&] { return context->signal.load(); });

      // when the timer end, play must end now.
      if (list[count].type == DYNAMIC) {
        context->signal = true;
      }
    });

    do {
      playWallPaper(&list[count]);
    } while (list[count].type == DYNAMIC && !context->signal);

    timer.join();
  };

  while (true) {
    playWallPaperOnce();

    if (!tasklist.empty()) {
      handleTask();
    } else {
      if (!context->loop) {
        next();
      }
    }
    context->signal = false;

    if (context->currentList.empty() || !context->testPaper.empty()) {
      break;
    } else if (context->currentList != list.name) {
      break;
    }
  }
}

void Player::play() {
  if (!context->testPaper.empty()) {
    testWallPaper();

  } else {
    auto& list = wplists_play[context->currentList];
    if (list.size() > 0) {
      playList(list);
    } else {
      LOG(INFO) << "The number of wallpapers for " << context->currentList
                << " is 0. Play another set of "
                   "wallpapers.";
      for (const auto& [name, list] : wplists_play) {
        if (list.size() > 0) {
          context->currentList = name;
          break;
        }
      }
    }
  }
}

bool Player::init(wpList startList) {
  if (context->currentList.empty()) {
    context->currentList = startList.name;
  }

  return renderer->initRenderer();
}

Player::Player(TaskList& tasklist,
               std::unordered_map<std::string, wpList>& wplists)
    : tasklist(tasklist), wplists_resource(wplists) {
  context = std::make_unique<PlayerContext>();
  decoder = std::make_unique<Decoder>(context->signal, context->frameBuffer);
  renderer = std::make_unique<Renderer>(context->signal, context->frameBuffer);

  context->signal = false;
  wplists_play = wplists_resource;
}

void Player::notify() {
  context->signal = true;
  context->cv.notify_all();
}

std::string Player::play_info() {
  std::string info;
  if (!context->testPaper.empty()) {
    info += "\nCurrent WallPaper: ";
    info += context->testPaper;
    return info + "\n";
  }

  info += "\nCurrent List: " + context->currentList;

  info += "\nCurrent WallPaper: ";
  auto wallpaper =
      wplists_play[context->currentList][context->currentWallPaper];
  info += wallpaper.name;

  info += "\nCurrent Mode: ";
  if (context->loop)
    info += "manual";
  else
    info += "auto";

  return info + "\n";
}
