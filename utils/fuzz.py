import random
import os
from pathlib import Path

OUTPUT_DIR = "vix_fuzz_out"  # 输出文件夹
FILE_COUNT = 50             # 生成50个文件
LOOP_LIMIT = 8              # 循环上限，避免死循环

# 创建输出目录
Path(OUTPUT_DIR).mkdir(exist_ok=True)

# 基础类型池
BASE_TYPES = ["i32", "f64", "bool", "string"]
# 安全运算符
SAFE_OPS = ["+", "-", "*", "/", "==", "!=", "<", ">", "<=", ">="]
# 字面量生成
def rand_literal():
    kind = random.choice(["int", "float", "bool", "str"])
    if kind == "int":
        return str(random.randint(1, 30))
    elif kind == "float":
        return f"{random.uniform(1.0, 20.0):.1f}"
    elif kind == "bool":
        return random.choice(["true", "false"])
    else:
        return f'"fuzz_msg_{random.randint(1, 99)}"'

#变量名
def new_var(prefix="var"):
    return f"{prefix}_{random.randint(10, 99)}"

# 变量声明
def gen_var_decl():
    v_type = random.choice(BASE_TYPES)
    var_name = new_var()
    val = rand_literal()
    return random.choice([
        f"let {var_name}: {v_type} = {val}",
        f"mut {var_name}: {v_type} = {val}"
    ])

# 赋值
def gen_safe_assign():
    var = new_var("num")
    val = random.randint(1, 20)
    return f"mut {var}: i32 = {val}\n    {var} += 2"

# if
def gen_if():
    a = random.randint(1, 20)
    b = random.randint(1, 20)
    cond = f"{a} > {b}"
    msg = rand_literal()
    return f"if ({cond}) {{\n        print({msg})\n    }}"

# for
def gen_for():
    i_var = new_var("i")
    end = random.randint(4, LOOP_LIMIT)
    return f"for ({i_var} in 1 .. {end}) {{\n        print({i_var})\n    }}"

# while
def gen_while():
    cnt = new_var("cnt")
    return f"""mut {cnt}: i32 = 0
    while ({cnt} < {LOOP_LIMIT-2}) {{
        print("while step")
        {cnt} += 1
    }}"""

# 简单结构体
# todo!
def gen_struct():
    return """struct Point {
    x: i32,
    y: f64
}"""

# 简单函数
# todo!
def gen_helper_func():
    func_name = new_var("calc")
    return f"""fn {func_name}(a: i32, b: i32): i32 {{
    return a + b
}}"""

# 生成 main 函数内部语句
def gen_main_body():
    stmt_pool = [
        gen_var_decl(),
        gen_safe_assign(),
        gen_if(),
        gen_for(),
        gen_while(),
        f'print("Vix fuzz test")'
    ]
    # 随机选4条语句
    # todo!
    return "\n    ".join(random.sample(stmt_pool, 4))

# 生成完整可运行 Vix 程序
def gen_vix_program():
    parts = []
    # 随机加结构体
    if random.random() < 0.4:
        parts.append(gen_struct())
    # 随机加辅助函数
    if random.random() < 0.3:
        parts.append(gen_helper_func())
    # main 函数
    main_fn = f"""fn main(): i32 {{
    {gen_main_body()}
    return 0
}}"""
    parts.append(main_fn)
    return "\n\n".join(parts)

# 批量生成50个文件
def batch_generate():
    print(f"开始生成 {FILE_COUNT} 个可运行 Vix 测试文件...")
    for idx in range(1, FILE_COUNT + 1):
        filename = f"fuzz_{idx:03d}.vix"
        filepath = os.path.join(OUTPUT_DIR, filename)
        code = gen_vix_program()
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(code)
        print(f"已生成: {filename}")
    print(f"\n✅ 全部完成！文件位于: {OUTPUT_DIR}/")

if __name__ == "__main__":
    batch_generate()