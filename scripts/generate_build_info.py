import datetime
import os
import subprocess

Import("env")


def shell_value(args, fallback):
    try:
        return subprocess.check_output(args, stderr=subprocess.DEVNULL, text=True).strip()
    except Exception:
        return fallback


def c_string(value):
    return value.replace("\\", "\\\\").replace('"', '\\"')


build_dir = env.subst("$BUILD_DIR")
generated_dir = os.path.join(build_dir, "generated")
os.makedirs(generated_dir, exist_ok=True)

project_dir = env.subst("$PROJECT_DIR")
version_script = os.path.join(project_dir, "scripts", "version-string.sh")
firmware_version = shell_value([version_script], "0.0.0-dev")

git_hash = shell_value(["git", "rev-parse", "--short=12", "HEAD"], "unknown")
build_time = datetime.datetime.utcnow().replace(microsecond=0).isoformat() + "Z"
build_env = env.subst("$PIOENV") or "unknown"

header_path = os.path.join(generated_dir, "BuildInfoGenerated.h")
with open(header_path, "w", encoding="utf-8") as f:
    f.write("#pragma once\n\n")
    f.write(f'#define VERSION_STRING "{c_string(firmware_version)}"\n')
    f.write(f'#define BUILD_GIT_HASH "{c_string(git_hash)}"\n')
    f.write(f'#define BUILD_TIME "{c_string(build_time)}"\n')
    f.write(f'#define BUILD_ENV "{c_string(build_env)}"\n')

env.Append(CPPPATH=[generated_dir])
