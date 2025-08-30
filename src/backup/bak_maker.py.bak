from pathlib import Path
import subprocess
import argparse
import shutil
import sys
import os

def exe_extension():
    if sys.platform == 'win32':
        return '.exe'
    else:
        return ''

def get_all_src_files() -> list[Path]:
    all_files = []
    for subdir, dirs, files in os.walk(Path.cwd() / 'src'):
        for file in files:
            all_files.append(Path(subdir) / file)
    return all_files

def build() -> None:
    (Path.cwd() / 'bin').mkdir(exist_ok=True)
    subprocess.run(['pyinstaller','--noconfirm','--onefile','--console',*get_all_src_files()],cwd=Path.cwd())
    shutil.copy2(Path.cwd() / 'dist' / ('buildit' + exe_extension()),Path.cwd() / 'bin')
    clean()

def test() -> None:
    subprocess.run(['buildit','-t','cpp','test'],cwd=Path.cwd())
 
def build_and_test() -> None:
    build()
    test()

def clean() -> None:
	shutil.rmtree(Path.cwd() / 'dist',ignore_errors=True)
	shutil.rmtree(Path.cwd() / 'build',ignore_errors=True)
	os.remove(Path.cwd() / 'buildit.spec')
 
parser = argparse.ArgumentParser(prog='maker',description='makes',epilog='')
parser.add_argument('command')

args = parser.parse_args()

if(args.command == 'test'):
    test()
if(args.command == 'build'):
    build()
if(args.command == 'build-and-test'):
    build_and_test()