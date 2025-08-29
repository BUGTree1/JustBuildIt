import config;
import utils;
import args;

version(Windows){
    bool platform_windows = true;
} else {
    bool platform_windows = false;
}

int main(string[] args){
    Args_info args_info = parse_args(args);
    run_flags(args_info);

    //string config_path = slash(args_info.project_dir , "buildme.yaml");
    //Config_Parsed cfg = parse_config(config_path);
    return 0;
}