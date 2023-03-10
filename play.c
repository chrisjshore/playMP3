#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

struct termios origterm;

void usage(){
    printf("Usage: play [options] <path/to/audio/file> \n");
    printf("Options:\n");
    printf(" --help\t\tPrint usage\n");
    printf(" -h\t\tSame as above\n");
    printf("Commands:\n");
    printf(" q\t\tQuit the program\n");
    printf(" [spacebar]\tPause or resume playing\n");
}

void reset_terminal(){
  tcsetattr(STDIN_FILENO, TCSANOW, &origterm);
  printf("\n");
}

void* command_thread(void* sound){
  ma_sound* pSound = sound;
  char c;

  while(!pSound->atEnd){
    c = getchar();
	  
	  switch(c) {
	    case 'q': {
		    ma_sound_stop(pSound);
		    putchar(c);
        /*potential memory leak since we don't uninit the engine before exiting*/
        /*TODO: get pSound's engine and uninit*/
        exit(0);
	    }
	    case ' ': {
		    if(ma_sound_is_playing(pSound)){
		      ma_sound_stop(pSound);
		    }
		    else {
		      ma_sound_start(pSound);
		    }
		    break;
	    }
    }
  }
  pthread_exit(0);
}

int main(int argc, char** argv){
    ma_result result;
    ma_engine engine;
    ma_sound  sound;
    ma_uint32 soundFlags;
    ma_node*  node;
    //ma_fence fence;
    static struct termios newterm;

    if(argc < 2){
        printf("no input file...\n");
        usage();
        return -1;
    }
    
    if((strcmp(argv[1],"--help") == 0) || (strcmp(argv[1],"-h") == 0)){
    	usage();
	    return 0;
    }

    result = ma_engine_init(NULL, &engine);
    if(result != MA_SUCCESS){
        return -1;
    }

    node = ma_node_graph_get_endpoint(&engine.nodeGraph);

    soundFlags |= MA_SOUND_FLAG_ASYNC;
    soundFlags |= MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT;
    soundFlags |= MA_SOUND_FLAG_NO_PITCH;
    soundFlags |= MA_SOUND_FLAG_NO_SPATIALIZATION;
    
    result = ma_sound_init_from_file(&engine, argv[1], soundFlags, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        printf("failed to init file\n");
        return -1;
    }
     
    result = ma_node_attach_output_bus(&sound, 0, node, 0);
    if(result != MA_SUCCESS){
    	printf("failed to attach node to output bus\n");
	    return -1;
    }

    /*TODO: research what the fence does, seems to clip higher frequencies...*/
    //result = ma_fence_init(&fence);
    //if(result != MA_SUCCESS){
    //    printf("failed to init fence\n");
    //}    
    //ma_fence_wait(&fence);

    result = ma_sound_start(&sound);
    if (result != MA_SUCCESS) {
        printf("failed to start sound\n");
	      return -1;
    }
    printf("playing...");
    
    /*doing some terminal magic so we don't have to press enter for commands to get caught*/
    tcgetattr(STDIN_FILENO, &origterm);
    atexit(reset_terminal);
    newterm = origterm;
    newterm.c_lflag &= ~(ICANON | ECHO );
    tcsetattr(STDIN_FILENO, TCSANOW, &newterm);
    
    pthread_t cmdThread;
    pthread_create(&cmdThread, NULL, &command_thread, &sound);

    while(!sound.atEnd){
      sleep(2);
    }

    /*if we get this far then cancel the other thread*/
    pthread_cancel(cmdThread);
    ma_engine_uninit(&engine);

    return 0;
}
