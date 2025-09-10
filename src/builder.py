from pathlib import Path
import utils

def build(config : dict, args : dict):
    project_dir = args['project_dir']
    
    if utils.pkgconf_available:
        for pkgconf_lib in config['pkgconf_libs']:
            pkgconf_out = utils.run([utils.pkgconf_path, '--libs', '--cflags' ,pkgconf_lib], True, True, project_dir)
            pkgconf_parsed = utils.parse_pkgconf(pkgconf_out) # type: ignore
            config['libs']         += pkgconf_parsed['libs']
            config['lib_paths']     += pkgconf_parsed['lib_dirs']
            config['include_paths'] += pkgconf_parsed['include_dirs']
            config['flags']        += pkgconf_parsed['flags']
            
    for exec_pre in config['exec_prebuild']:
        utils.run(exec_pre, False, True, project_dir)
        
    if Path(config['output_dir']).is_absolute():
        whole_out_dir: Path  = Path(config['output_dir'])
    else:
        whole_out_dir: Path = (project_dir / Path(config['output_dir'] / Path(utils.os_triplet))).resolve()
        
    if not whole_out_dir.exists():
        utils.mkdir(whole_out_dir)
        
    if Path(config['object_dir']).is_absolute():
        whole_obj_dir: Path  = Path(config['object_dir'])
    else:
        whole_obj_dir: Path = (whole_out_dir / Path(config['object_dir'])).resolve()
        
    if not whole_obj_dir.exists():
        utils.mkdir(whole_obj_dir)
        
    if Path(config['source_dir']).is_absolute(): 
        whole_src_dir: Path  = Path(config['source_dir'])
    else:
        whole_src_dir: Path = (project_dir / Path(config['source_dir'])).resolve()
        
    whole_filename : str = config['file_name']
    
    output_runnable  = False
    output_extension = ''
    output_prefix    = ''
    if config['build_type'] == 'executable':
        output_runnable = True
        if utils.os_windows:
            output_extension = '.exe'
    elif config['build_type'] == 'static':
        output_prefix = 'lib'
        output_extension = '.a'
    elif config['build_type'] == 'dynamic':
        if utils.os_windows:
            output_extension = '.dll'
        else:
            output_extension = '.so'
    
    if config['auto_out_file_ext']:
        whole_filename = output_prefix + whole_filename + output_extension
        
    c_compiler = config['c_compiler']
    if c_compiler == '':
        c_compiler = config['compiler']
        
    cxx_compiler = config['cxx_compiler']
    if cxx_compiler == '':
        cxx_compiler = config['compiler']
       
    linker = config['linker'] 
    if linker == '':
        if cxx_compiler != '':
            linker = cxx_compiler
        else:
            linker = c_compiler
    
    last_build_time = 0
    if (whole_out_dir / whole_filename).exists():
        last_build_time = (whole_out_dir / whole_filename).stat().st_mtime
    
    all_src_files_to_compile: list[tuple[Path, Path, str]] = []
    all_obj_files: list[Path] = []
    for dir_path, dir_names, src_files in whole_src_dir.walk():
        for src_file in src_files:
            whole_src_file: Path = (dir_path / Path(src_file)).resolve()
            whole_obj_file: Path = (whole_obj_dir / whole_src_file.relative_to(whole_src_dir)).resolve().with_suffix('.o')
            if whole_src_file.suffix == '.c' or whole_src_file.suffix == '.cpp':
                if whole_src_file.stat().st_mtime > last_build_time:
                    all_src_files_to_compile.append((whole_src_file, whole_obj_file, whole_src_file.suffix))
                all_obj_files.append(whole_obj_file)
    
    needs_linking = False
    errors        = False
    for src_file, obj_file, file_suffix in all_src_files_to_compile:
        if file_suffix == '.cpp':
            file_compiler = cxx_compiler
        else:
            file_compiler = c_compiler
        
        cmd: list[str] = [file_compiler, utils.compiler_compile_only_flag, str(src_file), utils.compiler_output_name_flag, str(obj_file), *config['compiler_flags'], *config['flags']]
        for include_dir in config['include_dirs']:
            cmd.append(utils.compiler_include_dir_flag)
            cmd.append(include_dir)
        if utils.run(cmd, False, True, project_dir) != 0:
            errors = True
        else:
            needs_linking = True
    
    if needs_linking:
        if config['build_type'] == 'static':
            utils.run(['ar','rcs', str(whole_out_dir / whole_filename)] + list(map(str,all_obj_files)), False, True, project_dir)
        else:
            cmd: list[str] = [linker, utils.compiler_output_name_flag, str(whole_out_dir / whole_filename), *config['flags'], *config['linker_flags']] + list(map(str,all_obj_files))
            for lib_dir in config['lib_dirs']:
                cmd.append(utils.compiler_library_dir_flag)
                cmd.append(lib_dir)
            for lib in config['libs']:
                cmd.append(utils.compiler_library_flag)
                cmd.append(lib)
            if utils.run(cmd, False, True, project_dir) != 0:
                errors = True
            
    if config['run_after_build'] and (not errors) and output_runnable:
        utils.run([str(whole_out_dir / whole_filename), *config['run_args']], False, True, whole_out_dir)
    
    for exec_post in config['exec_postbuild']:
        utils.run(exec_post, False, True, project_dir)
        