package main

import (
	"log"
	"os/exec"
    "strings"
    "io"
)

var channel chan int = make(chan int);

func proc(args []string, work_dir string) {
    var cmd_to_run *exec.Cmd = exec.Command(args[0],args[1:]...);
    cmd_to_run.Dir = work_dir;

    log.Printf("$ %v",args);
    //log.Printf("PATH: %v",cmd_to_run.Path);
    //log.Printf("ARGS: %v",cmd_to_run.Args);
    
    std_out_pipe,_ := cmd_to_run.StdoutPipe();
    std_err_pipe,_ := cmd_to_run.StderrPipe();

    cmd_to_run.Start();
    std_out_builder := new(strings.Builder);
    io.Copy(std_out_builder,std_out_pipe);
    std_err_builder := new(strings.Builder);
    io.Copy(std_err_builder,std_err_pipe);
    log.Printf("%s",std_out_builder.String());
    log.Printf("%s",std_err_builder.String());
    cmd_to_run.Wait();
    channel <- cmd_to_run.ProcessState.ExitCode();
}

func main() {
    for i := 0; i < 10; i++ {
        go proc([]string{"ping","google.com"},".");
    }
    for i := 0; i < 10; i++ {
        <- channel
        //log.Printf("EXIT: %v",<- channel);
    }
    log.Println("main finished");
}
