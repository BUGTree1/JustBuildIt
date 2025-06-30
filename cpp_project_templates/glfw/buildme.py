create_vscode_settings = True
run_after_build        = True
proj_name              = "TEMPLATE"
proj_version           = "0.1.0.0"
file_name              = "TEMPLATE"
compiler               = "g++"
linker                 = "g++"
flags                  = ['-Wall','-Wextra','-O3']
compiler_flags         = []
linker_flags           = []
output_path            = "bin"
source_path            = "src"
if platform_windows:
    libs               = ["glfw3","winmm","gdi32","user32","opengl32"]
else:
    libs               = ["glfw3"]
lib_paths              = []
include_paths          = []