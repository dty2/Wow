#!/usr/bin/python3

import importlib.util
import re
import subprocess
import sys
import time
from pathlib import Path

var_path = Path(__file__).resolve().parent.parent / "build" / "test" / "path.py"
spec = importlib.util.spec_from_file_location("var", var_path)
if spec is None or spec.loader is None:
    raise ImportError("Cannot load module from path.py")

var = importlib.util.module_from_spec(spec)
spec.loader.exec_module(var)

VARS = var.VARS

failed = "\033[91mFAILED\033[0m"
success = "\033[92mPASS\033[0m"


# expand ${VAR} to path
def expand_vars(text: str) -> str:
    for k, v in VARS.items():
        text = text.replace("${" + k + "}", v)
    return text


class TestGroup:
    def __init__(self):
        self.group = 0
        self.cmd = ""
        self.cmd_type = "cli"
        self.cmd_line = -1
        self.check: str | list[str]
        self.check_line = -1
        self.type = ""

    def run_daemon(self):
        cmd_expanded = expand_vars(self.cmd)
        subprocess.Popen(
            cmd_expanded,
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

    def run_shell(self):
        cmd_expanded = expand_vars(self.cmd)
        subprocess.run(["sh", cmd_expanded], cwd=VARS["SRC"])

    def run_cli(self):
        cmd_expanded = expand_vars(self.cmd)
        res = subprocess.run(cmd_expanded, shell=True, text=True, capture_output=True)
        output = res.stderr.strip()

        if res.returncode != 0:
            print(f"Test {failed}: Run '{self.cmd}' error")
            return False

        if self.type == "":
            return True

        if self.type == "file":
            if isinstance(self.check, list):
                path_str = "".join(self.check)
            else:
                path_str = self.check
            file = Path(expand_vars(path_str))

            if not file.exists():
                print(f"Test {self.cmd:<50} {failed}")
                print(f"{file} not exist")
                return False
            else:
                print(f"Test {self.cmd:<50} {success}")
                return True
        elif self.type == "text":
            check = "".join(self.check)
            if not re.search(re.escape(check), output, re.DOTALL):
                print(f"Test {self.cmd:<50} {failed}")
                print("Expected pattern:")
                print(check)
                print("Actual output:")
                print(output)
                return False
            else:
                print(f"Test {self.cmd:<50} {success}")
                return True
        elif self.type == "regex":
            check = "".join(self.check)
            if not re.search(check, output, re.DOTALL):
                print(f"Test {self.cmd:<50} {failed}")
                print("Expected pattern:")
                print(check)
                print("Actual output:")
                print(output)
                return False
            else:
                print(f"Test {self.cmd:<50} {success}")
                return True

    def run(self):
        if self.cmd_type == "cli":
            return self.run_cli()
        elif self.cmd_type == "daemon":
            self.run_daemon()
            return True
        elif self.cmd_type == "shell":
            return self.run_shell()


def get_groups(lines, groups):
    status = True
    current = None

    count = 1
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line.startswith("# RUN:"):
            if current != None:
                if current.cmd != "":
                    groups.append(current)
                else:
                    print(f"Syntax error at {current.check_line}: no cmd for check.")
                    status = False
            current = TestGroup()

            if line.endswith("CLI"):
                current.cmd_type = "cli"
            elif line.endswith("DAEMON"):
                current.cmd_type = "daemon"
            elif line.endswith("SHELL"):
                current.cmd_type = "shell"
            else:
                print(f"Syntax error at {i}: cmd format error")
                i += 1
                continue

            current.group = count
            count += 1
            i += 1
            current.cmd = lines[i].strip()

        elif line.startswith("# CHECK:"):
            if current == None:
                status = False
                current = TestGroup()

            current.check_line = i
            i += 1

            if line.endswith("FILE"):
                current.type = "file"
                current.check = []
                while i < len(lines) and not lines[i].startswith("#"):
                    current.check.append(lines[i].strip())
                    i += 1

                i -= 1

            if line.endswith("TEXT"):
                current.type = "text"
                current.check = ""
                buf = []
                while i < len(lines) and not lines[i].startswith("#"):
                    buf.append(lines[i].rstrip("\n"))
                    i += 1

                i -= 1
                current.check = "\n".join(buf)

            if line.endswith("REGEX"):
                current.type = "regex"
                current.check = lines[i].strip()

        else:
            print(f"Syntax error at {i}: Unknown syntax")
            print(f"Line: {line}")
            status = False

        i += 1

    if current != None:
        groups.append(current)

    return status


def main():
    if len(sys.argv) < 2:
        print("Usage: runtest.py <testfile>")
        sys.exit(1)

    test_file = Path(sys.argv[1])
    if not test_file.exists():
        print(f"Test file not found: {test_file}")
        sys.exit(1)

    lines = test_file.read_text().splitlines(True)
    groups = []
    status = True
    if get_groups(lines, groups) == False:
        sys.exit(1)
    else:
        for group in groups:
            if group.run() == False:
                status = False
            else:
                # It needs to sleep for a while
                # otherwise the following commands will be executed too quickly
                time.sleep(0.5)

    if status == False:
        sys.exit(1)

    sys.exit(0)


try:
    main()
except KeyboardInterrupt:
    print("Test interrupted by user (Ctrl+C)")
