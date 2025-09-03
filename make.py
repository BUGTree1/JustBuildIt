from pathlib import Path
import subprocess
import argparse
import shutil
import os

def run(cmd,cwd = None):
    print('$ ',end='')
    print(cmd)
    subprocess.run(cmd,cwd=cwd)

def clean():
	shutil.rmtree(Path.cwd() / 'dist',ignore_errors=True)
	shutil.rmtree(Path.cwd() / 'build',ignore_errors=True)
	os.remove(Path.cwd() / 'buildit.spec')

def build():
    src_files = []
    src_glob = Path('src').glob('**/*')
    for src_file in src_glob:
        if src_file.suffix == '.py':
            src_files.append(src_file)
    
    run(['pyinstaller','-n','buildit','--noconfirm','--console','--onefile','--optimize','2',*src_files],cwd=Path.cwd())
    
    if not (Path.cwd() / 'bin').exists():
        (Path.cwd() / 'bin').mkdir()
    
    if (Path.cwd() / 'dist' / 'buildit.exe').exists():
        shutil.copy2(Path.cwd() / 'dist' / 'buildit.exe',Path.cwd() / 'bin')
    elif (Path.cwd() / 'dist' / 'buildit').exists():
        shutil.copy2(Path.cwd() / 'dist' / 'buildit',Path.cwd() / 'bin')
    else:
        print('ERROR: No binary found!')
        exit(1)
    
    clean()

def test():
    run(['python','src/main.py'],cwd=Path.cwd())
    run(['python','src/main.py','-h'],cwd=Path.cwd())
    run(['python','src/main.py','-t','cpp','test'],cwd=Path.cwd())
 
parser = argparse.ArgumentParser(prog='make',description='makes',epilog='')
parser.add_argument('command')

args = parser.parse_args()

if(args.command == 'test'):
    test()
if(args.command == 'build'):
    build()