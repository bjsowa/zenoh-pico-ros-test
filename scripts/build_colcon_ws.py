#!/usr/bin/env python3

import os
import shutil
from pathlib import Path
import subprocess
import argparse


def main():
    project_dir = Path(__file__).resolve().parent.parent
    colcon_ws_dir = project_dir / "colcon_ws"

    parser = argparse.ArgumentParser(description="Micro-ROS Build Script")
    parser.add_argument(
        "-d", "--debug", action="store_true", help="Build in Debug mode"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose output"
    )
    parser.add_argument(
        "-x",
        "--no-clean",
        action="store_false",
        dest="clean",
        help="Do not clean colcon workspace",
    )
    args = parser.parse_args()

    build_type = "Debug" if args.debug else "Release"
    verbose_makefile = "ON" if args.verbose else "OFF"
    event_handlers = "console_cohesion+" if args.verbose else "console_stderr-"

    if args.clean:
        print("--> Cleaning colcon workspace...")
        shutil.rmtree(colcon_ws_dir / "build", ignore_errors=True)
        shutil.rmtree(colcon_ws_dir / "install", ignore_errors=True)
        shutil.rmtree(colcon_ws_dir / "log", ignore_errors=True)
        shutil.rmtree(colcon_ws_dir / "src", ignore_errors=True)

    print("--> Importing packages...")

    os.makedirs(colcon_ws_dir / "src", exist_ok=True)
    os.chdir(colcon_ws_dir)

    vcs_import_cmd = [
        "vcs",
        "import",
        "--input",
        str(colcon_ws_dir / "packages.repos"),
        "src",
    ]
    subprocess.run(vcs_import_cmd, check=True)

    print("--> Building colcon workspace...")

    colcon_ignore_packages = [
        "lttngpy",
        "rcl_lifecycle",
        "rcl_logging_noop",
        "rcl_logging_spdlog",
        "rcl_yaml_param_parser",
        "rclc_examples",
        "rclc_lifecycle",
        "rclc_parameter",
        "ros2trace",
        "rosidl_cli",
        "rosidl_generator_cpp",
        "rosidl_runtime_cpp",
        "rosidl_typesupport_cpp",
        "rosidl_typesupport_introspection_c",
        "rosidl_typesupport_introspection_cpp",
        "rosidl_typesupport_introspection_tests",
        "rosidl_typesupport_microxrcedds_cpp",
        "rosidl_typesupport_microxrcedds_c_tests",
        "rosidl_typesupport_microxrcedds_test_msg",
        "rosidl_typesupport_tests",
        "test_msgs",
        "test_rmw_implementation",
        "test_ros2trace",
        "test_tracetools",
        "test_tracetools_launch",
        "tracetools_launch",
        "tracetools_read",
        "tracetools_test",
        "tracetools_trace",
    ]

    os.environ["RMW_IMPLEMENTATION"] = "rmw_zenohpico_c"

    colcon_build_cmd = [
        "colcon",
        "build",
        "--merge-install",
        "--metas",
        str(colcon_ws_dir / "colcon.meta"),
        "--event-handlers",
        event_handlers,
        "--cmake-args",
        "--no-warn-unused-cli",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DBUILD_TESTING=OFF",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        f"-DCMAKE_VERBOSE_MAKEFILE={verbose_makefile}",
        # f"-DCMAKE_TOOLCHAIN_FILE={toolchain_path}",
        "--packages-ignore",
        *colcon_ignore_packages,
    ]
    subprocess.run(colcon_build_cmd, check=True)


if __name__ == "__main__":
    main()
