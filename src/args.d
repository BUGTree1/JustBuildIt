module args;

import commandr;
import buildit;
import utils;

struct Args_info{
    string buildit_dir;
    string templates_dir;
    string project_dir;

    string template_name;

    string init_remote_url;
    string push_commit_name;

    bool clean;
    bool rebuild;
    bool pull;
    bool discard; 
}

Args_info parse_args(string[] args){
    Args_info info;
    info.buildit_dir = thisExePath().dirName;
    info.templates_dir = slash(info.buildit_dir,"..","project_templates");

    string available_templates_string;

    foreach(string template_dir; dirEntries(info.templates_dir, SpanMode.shallow)) {
        available_templates_string ~= template_dir.baseName ~ " ";
    }

    HelpOutput helpConfig = HelpOutput.init;
    helpConfig.colors = true;
    helpConfig.indent = 30;

    // FLAG - is or is not e.g. -h
    // OPTION - takes optional argument e.g. -template <template>
    // ARGUMENT - needs positional argument e.g. <PROJECT PATH>
    ProgramArgs prog = new Program("JustBuildIt", "v0.1")
        .summary(format("Simple D build system for C/C++ with templates: %s",available_templates_string))
        .author("BUGTree_")
        .add(new Flag("c", "clean", "Remove all built files"))
        .add(new Flag("r", "rebuild", "Rebuild project from scrach"))
        .add(new Flag("l", "pull", "(-l/load) Pull all new changes to the remote repo"))
        .add(new Flag("d", "discard", "Discard all changes and pull newest remote repo"))
        .add(new Option("t", "template", "Generate template with name").tag("<template name>"))
        .add(new Option("i", "init", "Initialize a git repo with remote").tag("<remote url>"))
        .add(new Option("s", "push", "(-s/save) push to remote repo with commit name").tag("<commit name>"))
        .add(new Argument("project_dir", "Path to the project (if not specified will use current working directory)").optional())
        .parse(args,helpConfig);


    info.project_dir = slash(prog.arg("project_dir"));

    info.template_name = prog.option("template");
    info.init_remote_url = prog.option("init");
    info.push_commit_name = prog.option("push");

    info.clean = prog.hasFlag("clean");
    info.rebuild = prog.hasFlag("rebuild");
    info.pull = prog.hasFlag("pull");
    info.discard = prog.hasFlag("discard");
    return info;
}

bool confirm_option(){
    writeln("you sure? (yes or no):");
    string input = readln();
    return toLower(input).startsWith("yes") == true;
}

void run_flags(Args_info info){
    if(info.clean){
        string bin_dir = slash(info.project_dir,"bin");
        writefln("CLEANING: %s (will remove %s)",info.project_dir,bin_dir);
        if(confirm_option()){
            writefln("REMOVING: %s",bin_dir);
            rmdirRecurse(bin_dir);
        }
        exit(0);
    }

    if(info.init_remote_url != ""){
        writefln("Initializing GIT repository with remote: %s",info.init_remote_url);
        util_exec("git", ["-i"],                                            info.project_dir);
        util_exec("git", ["remote","set-url","origin",info.init_remote_url],info.project_dir);
        exit(0);
    }

    if(info.push_commit_name != ""){
        writefln("Pushing commit with name %s to GIT repo",info.push_commit_name);
        util_exec("git", ["commit","-m",info.push_commit_name],info.project_dir);
        util_exec("git", ["push","origin","main"],             info.project_dir);
        exit(0);
    }

    if(info.pull){
        writeln("Pulling all changes from GIT remote");
        util_exec("git", ["pull"],info.project_dir);
        exit(0);
    }

    if(info.discard){
        writeln("Discarding all changes and pulling from GIT remote");
        if(confirm_option()){
            util_exec("git", ["fetch","origin"],                   info.project_dir);
            util_exec("git", ["reset","--hard" ,"origin/master"]  ,info.project_dir);
        }
        exit(0);
    }

    if(info.template_name != ""){
        string template_dir = slash(info.templates_dir,info.template_name);
        writefln("Copying template %s from %s",info.template_name,template_dir);
        writefln("to %s",info.project_dir);

        mkdirRecurse(info.project_dir);
        foreach (string template_file; dirEntries(template_dir, SpanMode.depth)) {
            if (isFile(template_file)){
                //TODO: copies all files to one diectory ignoring template subdirs
                std.file.copy(template_file,slash(info.project_dir,template_file.baseName));
            }
        }

        exit(0);
    }
}