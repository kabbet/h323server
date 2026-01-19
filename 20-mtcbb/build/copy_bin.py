from pathlib import Path
import shutil
import sys

def copy_file(src, dst):
    # 如果目标路径不存在，创建它
    dst.mkdir(parents=True, exist_ok=True)

    # 判断源路径是文件还是目录
    if src.is_dir():
        # 如果是目录，使用 copytree
        shutil.copytree(src, dst)
    else:
        # 如果是文件，使用 copy，并确保目标路径包含文件名
        dst_file = dst / src.name
        shutil.copy(src, dst_file)

if __name__ == "__main__":
    # 获取命令行参数
    p = Path(sys.argv[1])
    flag = sys.argv[2]  # 目前未使用该参数
    print(f"flag: {flag}")
    oldPath = p.resolve()
    dstPath = Path(oldPath).parents[4] / "10-common/version/bin"
    
    # 修改目标路径，根据 flag 值决定是 debug 还是 release
    if flag == "debug":
        dstPath = dstPath / "debug"
    else:
        dstPath = dstPath / "release"

    # 调用拷贝函数
    copy_file(oldPath, dstPath)
    print(f"Copied {oldPath} to {dstPath}")
