from pathlib import Path
import concurrent.futures
import multiprocessing
import threading
import utils
import time

loading_wheel:        int             = 0
loading_wheel_time:   int             = 0
threads_count:        int             = 0
completed_threads:    int             = 0
threads_lock:         threading.Lock  = threading.Lock()
compiling_stop_event: threading.Event = threading.Event()

def compiling_progressbar():
    global completed_threads
    global threads_count
    global threads_lock
    global loading_wheel
    global loading_wheel_time
    global compiling_stop_event
    
    completed = 0
    wheel_speed = 0.1
    nanoseconds_to_seconds = 1_000_000_000
    wheel_speed_ns = wheel_speed * nanoseconds_to_seconds
    
    while not compiling_stop_event.is_set():
        with threads_lock:
            completed = threads_count
        
        perc = completed / threads_count
        perc *= 100
        
        
        if time.time_ns() - loading_wheel_time > wheel_speed_ns:
            loading_wheel_time = time.time_ns()
            loading_wheel += 1
        
        wheel = loading_wheel % 4
        wheel_char = "-\\|/"[wheel]
        
        print(f"Done Compiling: {completed}/{threads_count} {perc}% {wheel_char}", end='\r')

def get_object_from_source(src_file: Path, src_dir: Path, obj_dir: Path) -> Path:
    return (obj_dir / src_file.relative_to(src_dir)).resolve().with_suffix('.o')

def collect_source_files(src_dir: Path, last_build_time: float) -> list[list[Path]]:
    all_src_files            : list[Path] = []
    all_src_files_to_compile : list[Path] = []
    for dir_path, dir_names, src_files in src_dir.walk():
        for src_file in src_files:
            whole_src_file: Path = (dir_path / Path(src_file)).resolve()
            if whole_src_file.suffix == '.c' or whole_src_file.suffix == '.cpp':
                if whole_src_file.stat().st_mtime > last_build_time:
                    all_src_files_to_compile.append(whole_src_file)
                all_src_files.append(whole_src_file)
    return [all_src_files, all_src_files_to_compile]

# thread.get() == 0
def compile_file(project_dir: Path, src_file: Path, out_file: Path, compiler: str, include_dirs: list[str], flags: list[str]) -> int:
    print(f'Compiling file: {src_file} ...')
    
    cmd: list[str] = [compiler, utils.compiler_compile_only_flag, str(src_file), utils.compiler_output_name_flag, str(out_file)]
    if flags != None:
        cmd.extend(flags)
    for include_dir in include_dirs:
        cmd.append(utils.compiler_include_dir_flag)
        cmd.append(include_dir)
    out_code = utils.run(cmd, False, True, project_dir)
    
    global completed_threads
    with threads_lock:
        completed_threads += 1
    return out_code # type: ignore
       
def link_static(project_dir: Path, src_files: list[Path], out_file: Path) -> bool:
    print(f'Linking static: {out_file} ...')
    return utils.run(['ar','rcs', str(out_file)] + list(map(str,src_files)), False, True, project_dir) == 0
        
def link_dynamic(project_dir: Path, src_files: list[Path], out_file: Path, linker: str, library_dirs: list[str],libraries: list[str], flags: list[str]) -> bool:
    print(f'Linking dynamic: {out_file} ...')
    cmd: list[str] = [linker, utils.linker_shared_lib_flag, utils.compiler_output_name_flag, str(out_file)] + list(map(str,src_files))
    if flags != None:
        cmd.extend(flags)
    for lib_dir in library_dirs:
        cmd.append(utils.compiler_library_dir_flag)
        cmd.append(lib_dir)
    for lib in libraries:
        cmd.append(utils.compiler_library_flag)
        cmd.append(lib)
    
    return utils.run(cmd, False, True, project_dir) == 0
        
def link_executable(project_dir: Path, src_files: list[Path], out_file: Path, linker: str, library_dirs: list[str],libraries: list[str], flags: list[str]) -> bool:
    print(f'Linking executable: {out_file} ...')
    cmd: list[str] = [linker, utils.compiler_output_name_flag, str(out_file)] + list(map(str,src_files))
    if flags != None:
        cmd.extend(flags)
    for lib_dir in library_dirs:
        cmd.append(utils.compiler_library_dir_flag)
        cmd.append(lib_dir)
    for lib in libraries:
        cmd.append(utils.compiler_library_flag)
        cmd.append(lib)
    return utils.run(cmd, False, True, project_dir) == 0

def build(config : dict, args : dict):
    project_dir : Path = args['project_dir']
            
    for exec_pre in config['exec_prebuild']:
        utils.run(exec_pre, False, True, project_dir)
        
    if Path(config['output_dir']).is_absolute():
        whole_out_dir: Path  = Path(config['output_dir'])
    else:
        whole_out_dir: Path = (project_dir / Path(config['output_dir'] / Path(utils.os_triplet))).resolve()
        
    if Path(config['object_dir']).is_absolute():
        whole_obj_dir: Path  = Path(config['object_dir'])
    else:
        whole_obj_dir: Path = (whole_out_dir / Path(config['object_dir'])).resolve()
        
    if Path(config['source_dir']).is_absolute(): 
        whole_src_dir: Path  = Path(config['source_dir'])
    else:
        whole_src_dir: Path = (project_dir / Path(config['source_dir'])).resolve()
        
    c_compiler : str = config['c_compiler']
    cxx_compiler : str = config['cxx_compiler']
    linker : str = config['linker']
        
    if not whole_out_dir.exists():
        utils.mkdir(whole_out_dir)
        
    if not whole_obj_dir.exists():
        utils.mkdir(whole_obj_dir)
        
    whole_filename : str = config['file_name']
    
    output_runnable  : str = config['build_type'] == 'executable'
    output_extension : str = utils.os_file_extension[config['build_type']]
    output_prefix    : str = utils.os_file_prefix[config['build_type']]
    
    if config['auto_out_file_ext']:
        whole_filename = output_prefix + whole_filename + output_extension
    
    last_build_time : float = 0
    if (whole_out_dir / whole_filename).exists():
        last_build_time = (whole_out_dir / whole_filename).stat().st_mtime
    
    collected_src_files      : list[list[Path]] = collect_source_files(whole_src_dir, last_build_time)
    all_src_files            : list[Path] = collected_src_files[0]
    all_src_files_to_compile : list[Path] = collected_src_files[1]

    if ('rebuild' in args) and args['rebuild']:
        all_src_files_to_compile = all_src_files
    
    needs_linking = False
    errors        = False
    
    if len(all_src_files_to_compile) > 0:
        global threads_count
        threads_count = len(all_src_files_to_compile)
        
        bg_thread = threading.Thread(target=compiling_progressbar, daemon=True)
        bg_thread.start()
        
        with concurrent.futures.ThreadPoolExecutor(max_workers=multiprocessing.cpu_count()) as executor:
            futures: list[concurrent.futures.Future] = []
            for src_file in all_src_files_to_compile:
                if src_file.suffix == '.cpp' or src_file.suffix == '.cxx' or src_file.suffix == '.c++':
                    file_compiler = cxx_compiler
                else:
                    file_compiler = c_compiler
            
                futures.append(executor.submit(compile_file,project_dir, src_file, whole_out_dir / get_object_from_source(src_file,whole_src_dir,whole_obj_dir), file_compiler, config['include_dirs'], config['compiler_flags'].extend(config['flags'])))
            
            concurrent.futures.wait(futures)
        
            for future in futures:
                if future.result() != 0:
                    errors = True
        
        compiling_stop_event.set()         
        bg_thread.join()
            
        needs_linking = not errors
    
    if needs_linking:
        match config['build_type']:
            case 'executable':
                link_executable(project_dir,list(map(get_object_from_source,all_src_files,[whole_src_dir] * len(all_src_files),[whole_obj_dir] * len(all_src_files))),whole_out_dir / whole_filename, linker, config['lib_dirs'], config['libs'], config['linker_flags'].extend(config['flags']))
            case 'static':
                link_static(project_dir,list(map(get_object_from_source,all_src_files,[whole_src_dir] * len(all_src_files),[whole_obj_dir] * len(all_src_files))),whole_out_dir / whole_filename)
            case 'dynamic':
                link_dynamic(project_dir,list(map(get_object_from_source,all_src_files,[whole_src_dir] * len(all_src_files),[whole_obj_dir] * len(all_src_files))),whole_out_dir / whole_filename, linker, config['lib_dirs'], config['libs'], config['linker_flags'].extend(config['flags']))
            
            
    if config['run_after_build'] and (not errors) and output_runnable:
        print(f'Running output executable: {str(whole_out_dir / whole_filename)} ...')
        utils.run([str(whole_out_dir / whole_filename), *config['run_args']], False, True, whole_out_dir)
    
    for exec_post in config['exec_postbuild']:
        utils.run(exec_post, False, True, project_dir)
        