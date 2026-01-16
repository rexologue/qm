Собрать все это и запустить:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build
./build/my_prog
```

На Windows:

```bat
winget install --id Microsoft.VisualStudio.2022.BuildTools -e
```

И чтобы после установки точно были C++ компоненты (MSVC + SDK), открой установщик и поставь workload **“Desktop development with C++”**.

Дальше весь твой пайплайн по командам:

```bat
winget install --id Microsoft.VisualStudio.2022.BuildTools -e
```

```bat
winget install --id Ninja-build.Ninja -e
```

Открываешь **x64 Native Tools Command Prompt for VS 2022**, и выполняешь:

```bat
chcp 65001
```

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

```bat
cmake --build build
```

```bat
build\my_prog.exe
```
