from pathlib import Path
import shutil
import sys

def copy_file(src, dst, name):
    # 如果目标路径不存在，创建它
    dst.mkdir(parents=True, exist_ok=True)

    # 判断源路径是文件还是目录
    if src.is_dir():
        # 如果是目录，使用 copytree
        shutil.copytree(src, dst)
    else:
        # 如果是文件，使用 copy，并确保目标路径包含文件名
        dst_file = dst / name
        print(dst_file)
        shutil.copy(src, dst_file)

if __name__ == "__main__":
    # 获取命令行参数
    pin = Path(sys.argv[1])
    pout = Path(sys.argv[2])
    is_debug = True if sys.argv[3] == "debug" else False
    outname = pout.name
    oldPath = pin.resolve()
    dstPath = Path(oldPath).parents[4] / "10-common/lib/locallib/linux64/"

    # 修改目标路径，根据 flag 值决定是 debug 还是 release
    if is_debug:
        dstPath = dstPath / "debug"
    else:
        dstPath = dstPath / "release"

    # 调用拷贝函数
    copy_file(oldPath, dstPath, outname)
    print(f"Copied {oldPath} to {dstPath}")
