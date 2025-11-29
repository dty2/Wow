#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "decode/decoder.h"
#include "render/renderer.h"
#include "resource/wallpaper.h"

enum Action { Next = 0, Previous, Manual, Refresh, List, Test, Quit };

struct Task {
  Action action;
  std::string target;
  Task() = default;
  Task(std::string action, std::string target) {
    if (action == "next") {
      this->action = Next;
    } else if (action == "prev") {
      this->action = Previous;
    } else if (action == "manual") {
      this->action = Manual;
    } else if (action == "refresh") {
      this->action = Refresh;
    } else if (action == "list") {
      this->action = List;
    } else if (action == "test") {
      this->action = Test;
    } else if (action == "quit") {
      this->action = Quit;
    }
    this->target = target;
  }
};

class TaskList {
 private:
  std::mutex mtx;
  std::queue<Task> cmdQueue;
  std::condition_variable cv;

 public:
  bool empty();
  Task get();
  void push(std::string action, std::string target);
};

using oneFrame = std::vector<uint8_t>;
class FrameBuffer {
 private:
  std::queue<oneFrame> bufferQueue;
  std::mutex mtx;
  std::condition_variable cv;

 public:
  int size();
  bool empty();
  oneFrame get();
  void push(oneFrame frame);
  void clear();
};

struct PlayerContext {
  FrameBuffer frameBuffer;
  std::atomic_bool signal;
  std::mutex mtx;
  std::condition_variable cv;
  std::string testPaper;
  bool loop;
  int currentWallPaper;
  std::string currentList;
};

class Player {
 private:
  std::unique_ptr<PlayerContext> context;
  std::unordered_map<std::string, wpList> wplists_play;
  std::unordered_map<std::string, wpList>& wplists_resource;
  std::unique_ptr<Renderer> renderer;
  std::unique_ptr<Decoder> decoder;
  TaskList& tasklist;

  void next();
  void handleTask();
  void playWallPaper(WallPaper* wp);
  void playList(wpList& list);
  void testWallPaper();

 public:
  Player(TaskList& tasklist, std::unordered_map<std::string, wpList>& wplists);
  std::string play_info();
  void notify();
  bool init(wpList startList);

  void play();
};
