from pathlib import Path
import argparse
import utils

def parse() -> dict:
    templates_string : str = utils.string_subdirs(utils.templates_dir)
    
    formatter = lambda prog: argparse.HelpFormatter(prog,max_help_position=52)
    parser : argparse.ArgumentParser = argparse.ArgumentParser(formatter_class=formatter, prog='JustBuildIt', description='Simple build system for C/C++', epilog='Available default templates are: ' + templates_string)
    
    parser.format_help
    parser.add_argument('-t','--template', help='generate template', metavar='<template_name>')
    parser.add_argument('-r','--rebuild' , help='rebuild whole project', action='store_true')
    parser.add_argument('-i','--init'    , help='init a git repo with remote', metavar='<remote_url>')
    parser.add_argument('-s','--push'    , help='(-s - save) push to remote repo with commit name', metavar='<commit_name>')
    parser.add_argument('-l','--pull'    , help='(-l - load) pull lastest changes from remote repo', action='store_true')
    parser.add_argument('-d','--discard' , help='discard local changes and reset to remote repo', action='store_true')
    parser.add_argument('project_dir'    , help='Path to the project', default='.', nargs='?')
    
    parsed_args = parser.parse_args()
    
    compile_args: dict = {'project_dir' : Path(parsed_args.project_dir)}
    
    if parsed_args.template != None:
        src = utils.templates_dir / parsed_args.template
        dest = compile_args['project_dir']
        print(f'Copying template from: "{str(src)}" to: "{str(dest)}"')
        utils.copy(src,dest)
    
    if parsed_args.rebuild:
        compile_args['rebuild'] = True
    
    if parsed_args.init != None:
        utils.run(['git','init','-b','main']                       , False, False, compile_args['project_dir'])
        utils.run(['git','remote','add','origin', parsed_args.init], False, False, compile_args['project_dir'])
        utils.exit()
    if parsed_args.push != None:
        utils.run(['git','add','.']                     , False, False, compile_args['project_dir'])
        utils.run(['git','commit','-m',parsed_args.push], False, False, compile_args['project_dir'])
        utils.run(['git','push','origin','main']        , False, False, compile_args['project_dir'])
        utils.exit()
    if parsed_args.pull:
        utils.run(['git','pull','origin','main'], False, False, compile_args['project_dir'])
        utils.exit()
    if parsed_args.discard:
        utils.run(['git','reset','--hard']      , False, False, compile_args['project_dir'])
        utils.run(['git','pull','origin','main'], False, False, compile_args['project_dir'])
        utils.exit()
        
    return compile_args