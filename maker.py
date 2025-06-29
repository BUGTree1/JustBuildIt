from pathlib import Path
import subprocess
import argparse
import shutil
import os

def clean():
	shutil.rmtree(Path.cwd() / 'dist',ignore_errors=True)
	shutil.rmtree(Path.cwd() / 'build',ignore_errors=True)
	shutil.rmtree(Path.cwd() / 'test',ignore_errors=True)
	os.remove(Path.cwd() / 'buildit.spec')

def build():
    subprocess.run(['pyinstaller','--noconfirm','--onefile','--console','buildit.py'],cwd=Path.cwd())
    if (Path.cwd() / 'dist' / 'buildit.exe').exists():
        shutil.copy2(Path.cwd() / 'dist' / 'buildit.exe',Path.cwd())
    else:
        shutil.copy2(Path.cwd() / 'dist' / 'buildit',Path.cwd())
    clean()

def test():
    subprocess.run(['buildit','-t','cpp','test'],cwd=Path.cwd())
 
def build_and_test():
    build()
    test()
 
parser = argparse.ArgumentParser(prog='maker',description='makes',epilog='')
parser.add_argument('command')

args = parser.parse_args()

if(args.command == 'test'):
    test()
if(args.command == 'build'):
    build()
if(args.command == 'build-and-test'):
    build_and_test()