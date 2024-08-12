# zenoh-pico-ros-test

Create python virtual environment

```shell
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Build colcon workspace:
```shell
./scripts/build_colcon_ws.py
```

Build project using CMake:
```shell
mkdir build && cd build
cmake .. && cmake --build .
```