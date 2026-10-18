# todo-qt（墨迹清单 · Qt6）

本地待办，数据 `%APPDATA%/todo-qt/todos.json`（由 `QStandardPaths` 决定具体路径）。

```bash
cmake -B build -DCMAKE_PREFIX_PATH="<Qt6 msvc_64>"
cmake --build build --config Release
bash packaging/build.sh   # 绿色 zip + NSIS 安装包
```

安装包：`dist/墨迹清单-qt-<版本>-win64-setup.exe`。
