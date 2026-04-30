# /// script
# requires-python = ">=3.14"
# dependencies = [
#     "tqdm>=4.67.3",
# ]
# ///
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from tqdm import tqdm


def format_c():
    def format(file: Path):
        retry = 0
        _content = ""
        while True:
            content = file.read_text(
                encoding="utf-8",
                errors="ignore",
            )
            if _content == content:  # 格式化後保持不變即表示完成
                return file
            retry += 1
            if retry >= 10:  # 格式化一定次數後視為無盡的輪迴
                return file
            subprocess.run(
                [
                    "clang-format",
                    "-style",
                    "file",
                    "-i",
                    file,
                ],
                check=True,
            )
            _content = content

    print("正在使用 clang-format 格式化 C 相關檔案")

    # 檢查是否有 clang-format
    if shutil.which("clang-format") is None:
        print(
            "請安裝 LLVM(https://github.com/llvm/llvm-project/releases/)"
            " 並設定環境變數以使用 clang-format"
        )
        return

    # 對整個專案的 C 語言相關檔案格式化
    # 透過 .clang-format 設定格式化選項
    # 透過 .clang-format-ignore 指定不進行格式化的檔案
    files = [
        path
        for path in Path().rglob("*")
        if path.is_file() and path.suffix in [".c", ".cpp", ".h", ".hpp"]
    ]
    files.sort(  # 根據檔案大小進行排序，一般情況下越大的檔案需要的格式化時間越久，優先格式化
        key=lambda f: f.stat().st_size,
        reverse=True,
    )
    max_len = max([len(f.as_posix()) for f in files])
    with (
        ThreadPoolExecutor() as executor,  # 合理利用多核 CPU 同時對多個檔案進行格式化
        tqdm(  # 進度條
            executor.map(format, files),
            total=len(files),
            unit="file",
            postfix=f"{'':<{max_len}}",
        ) as progress_bar,
    ):
        for file in progress_bar:
            progress_bar.set_postfix_str(f"{file.as_posix():<{max_len}}")


def format_py():
    print("正在使用 uv 格式化 Python 檔案")

    # 檢查是否有 uv
    if shutil.which("uv") is None:
        print("請安裝 uv(https://docs.astral.sh/uv/getting-started/installation/)")
        return

    # 對整個專案的 Python 檔案格式化
    for cmd in [
        "uvx isort .",
        "uvx black .",
        "uv format --preview-feature format",
    ]:
        subprocess.run(
            cmd,
            check=True,
        )


def main():
    format_c()
    format_py()


if __name__ == "__main__":
    main()
