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


    info.project_dir = prog.arg("project_dir");

    info.template_name = prog.option("template");
    info.init_remote_url = prog.option("init_remote_url");
    info.push_commit_name = prog.option("push_commit_name");

    info.clean = prog.hasFlag("clean");
    info.rebuild = prog.hasFlag("rebuild");
    info.pull = prog.hasFlag("pull");
    info.discard = prog.hasFlag("discard");
    return info;
}

void run_flags(Args_info info){
    writefln("\n\nINFO: %s \n\n",info);
    
    // TODO: 
    // - clean
    // - rebuild 
    // - pull
    // - discard
    // - init
    // - push
}