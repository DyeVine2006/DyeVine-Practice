# Student Grade Management System (C)

一个基于 **单向链表** 的控制台学生成绩管理系统：程序启动时从文本文件载入全部记录，每次操作后自动写回文件，因此数据在多次运行之间不会丢失。

It keeps student records in a singly linked list, lists them in alphabetical order, and persists everything to a plain comma-separated text file.

---

## 功能一览

| 编号 | 功能 | 说明 |
| --- | --- | --- |
| 1 | 录入学生 | 按姓名**自动有序插入**（字典序），重名会被拒绝 |
| 2 | 查询学生 | 按姓名精确查找，未找到有明确提示 |
| 3 | 修改信息 | 可改姓名 / 性别 / 三门成绩；改成绩后**自动重算总分与平均分**；改名后链表**自动重排** |
| 4 | 删除学生 | 删除前**二次确认**，可取消 |
| 5 | 显示全部 | 按姓名排序输出全部记录并统计条数 |
| q | 退出 | 退出前数据已全部落盘 |

其他特性：

- **数据文件可指定**：命令行参数传入任意路径，默认当前目录的 `Grade.txt`
- **文件不存在不会崩**：自动按空列表启动，并在写回时创建文件
- **父目录自动创建**：支持多级不存在的目录（如 `out/2026/Grade.txt`）
- **容错解析**：跳过空行与损坏行（并给出警告），不会因一行脏数据而崩溃或丢全部记录
- **UTF-8 BOM 兼容**：记事本保存的带 BOM 文件不会被解析成乱码
- **输入校验**：成绩限定 0–100，性别限定 M/F，非法输入会提示并重试
- **不使用 `system()` / `cls`**：输出跨 Windows / Linux / macOS 一致

---

## 快速开始

### 1. 编译

需要任意一个 C 编译器。以 **MinGW-w64 (gcc)** 为例：

```bash
gcc -std=c11 -Wall -Wextra -o stu_manage_sys Stu_manage_sys.c
```

也可以直接用仓库里的 Makefile：

```bash
make          # 生成 stu_manage_sys
make clean    # 清理
```

### 2. 运行

```bash
./stu_manage_sys                    # 读写当前目录下的 Grade.txt
./stu_manage_sys data/Grade.txt     # 指定数据文件
./stu_manage_sys --help             # 查看用法
```

> **建议在项目目录下运行。** 默认数据文件是相对于**当前工作目录**的 `Grade.txt`；
> 如果你的编译产物在别的目录，请显式传入路径，例如
> `./build/stu_manage_sys ./data/Grade.txt`。

想用仓库里的样例数据试试：

```bash
cp data/Grade.txt Grade.txt
./stu_manage_sys
```

### 3. 在 IDE 里运行

| IDE | 操作 |
| --- | --- |
| **VS Code** | 安装 C/C++ 扩展 → 打开 `Stu_manage_sys.c` → 终端运行上面的 gcc 命令 |
| **Dev-C++** | 新建项目 → 加入 `Stu_manage_sys.c` → F11 编译运行 |
| **Visual Studio** | 新建空项目 → 添加源文件 → 若报 `fopen` 不安全，在项目属性里加 `_CRT_SECURE_NO_WARNINGS`（源码开头已定义） |

---

## 使用示例

主菜单：

```
Enter 1 to 5, or q to quit:
1: Input a student's information
2: Inquire a student's information
3: Modify a student's information
4: Delete a student's information
5: Display all the students' information
q: Quit (all changes are already saved)
>
```

**显示全部（按姓名排序）**

```
Name             Gender   English      Math         C       Sum   Average
-------------------------------------------------------------------------------
Amy              F         100.00     97.00     92.00    289.00     96.33
Chen Jie         F          95.00     97.00     89.00    281.00     93.67
David Smith      M          97.00     78.00     62.00    237.00     79.00
Oliver           M          89.00     76.00     65.00    230.00     76.67
Wu Binbin        M          86.00     92.00     79.50    257.50     85.83
-------------------------------------------------------------------------------
5 record(s) in total.
```

**查询**

```
> 2
Name to search: Amy
Name             Gender   English      Math         C       Sum   Average
-------------------------------------------------------------------------------
Amy              F         100.00     97.00     92.00    289.00     96.33
```

**修改成绩（自动重算总分 / 平均分）**

```
> 3
Name to modify: Wu Binbin
Current record:
Wu Binbin        M          86.00     92.00     79.50    257.50     85.83

1: Change the Name
2: Change the Gender
3: Change the Grade of English
...
Enter 1 to 5, or q: 3
New score: 95
Score updated:
Wu Binbin        M          95.00     92.00     79.50    266.50     88.83
```

---

## 数据文件格式

每行一条记录，**英文逗号分隔**，共 7 个字段：

```
姓名,性别,英语,数学,C语言,总分,平均分
Amy,F,100.000000,97.000000,92.000000,289.000000,96.333336
```

| 字段 | 说明 |
| --- | --- |
| 姓名 | 最多 19 个字符，不允许包含逗号，**不可重复** |
| 性别 | `M` 或 `F` |
| 三门成绩 | 0–100 的浮点数 |
| 总分 / 平均分 | 程序写回时输出；**读取时会重新计算，不信任文件中的旧值** |

> 姓名请不要包含逗号（否则会被当成字段分隔符）。文件请保存为 **UTF-8 无 BOM** 或 **ANSI**。

---

## 数据结构与实现要点

```
head(哨兵) -> [Amy] -> [Chen Jie] -> [David Smith] -> ... -> NULL
```

```c
typedef struct student {
    char name[20];
    char gender[3];
    float score_english, score_math, score_c;
    float sum, average;
    struct student *next;
} STU;
```

- **带哨兵头结点的单向链表**：插入 / 删除无需特判「操作第一个结点」，代码更简洁
- **有序插入**：`insert_sorted()` 用二级指针 `STU **link` 遍历，找到第一个字典序更大的结点后插入
- **查找返回前驱**：`find_student(head, name, &prev)` 同时给出前驱结点，删除时可直接摘链
- **文件持久化**：`load_records()` 用 `sscanf` 按 `%19[^,]` 解析逗号分隔字段；`save_records()` 用 `fprintf` 写回
- **整行读取**：所有交互统一用 `fgets` + 自己写的解析函数，避免 `scanf` 与 `getchar` 混用留下的残留字符问题

---

## 项目结构

```
Stu_manage_sys/
├── Stu_manage_sys.c     # 全部源码（约 850 行，分节注释）
├── Makefile             # make / make clean
├── data/
│   └── Grade.txt        # 样例数据（5 条记录）
├── LICENSE              # MIT
├── .gitignore
└── README.md
```

---

## 相比第一版修复了什么

初版能跑，但存在几个会影响别人试用的问题，这一版逐条修掉了：

| # | 问题 | 修复 |
| --- | --- | --- |
| 1 | 数据文件路径**写死** `D:\Grade.txt`，别人 clone 后必然打不开，且直接 `exit(0)` | 改为命令行参数 / 默认 `Grade.txt`；文件不存在时按空列表启动 |
| 2 | **每一轮循环都重写文件**（包括读失败的那次），且 `fopen` 失败即退出 | 只在有意义的操作后写回；写失败给出警告但不退出 |
| 3 | `MODIFY` 靠「摘链 → 改 → 重新插入」，若中途赋新名后会跳出重排循环，记录可能丢失 | 改名走「先改名再重新插入」，并显式重排 |
| 4 | `MODIFY` 中 `free()` 之后仍可能访问已释放结点 | 删除路径与修改路径彻底分离 |
| 5 | `scanf` + `getchar` 混用导致菜单残留字符被当成命令 | 全部改为整行读取 + 显式解析 |
| 6 | `DISPLAY` 靠 `strlen(name) < 8` 切换制表符来对齐，长名字会错位 | 统一用 `%-16s%9.2f` 宽度格式化 |
| 7 | 提示语复制粘贴错误（`MODIFY` 里问 "inquire"） | 文案逐条校正 |
| 8 | 无输入校验：成绩可填 999，性别可填任意字符 | 成绩限 0–100，性别限 M/F，非法输入重试 |
| 9 | 空数据文件 / 全空列表时行为不明确 | 统一的空列表处理，显示 0 条 |
| 10 | 无 README、无样例数据、无 License | 本仓库补齐 |

> 初版源码仍保留在 Git 历史中（提交 `ee5bd3e`），可通过
> `git show ee5bd3e:"Stu_manage_sys/Student Grade Management System.c"` 查看。

---

## 已知限制

- 数据全部驻留内存，适合课程规模的数据；记录数极大时链表查找为 O(n)，可改为哈希表或平衡树
- 姓名精确匹配，暂不支持模糊 / 拼音搜索
- 单文件顺序写回（每次操作重写整个文件），数据量很大时可改为「只追加 + 定期整理」

---

## License

[MIT](LICENSE) © 2026 DyeVine
