#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <error.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int myevent(SDL_Event *e);

int main(int argc,char** argv)
{

  char name[1000];
  if(SDL_Init(SDL_INIT_EVERYTHING)==-1)
    return -1;
  IMG_Init(IMG_INIT_JPG);
  SDL_Window* mywin=SDL_CreateWindow("play",0,0,1366,768,SDL_WINDOW_SHOWN);
  SDL_Renderer* myrender=SDL_CreateRenderer(mywin,-1,SDL_RENDERER_ACCELERATED);
  SDL_ShowWindow(mywin);
  av_register_all();
  AVFormatContext *pFormatCtx=NULL;
  AVInputFormat fmt_input;
  
  if(avformat_open_input(&pFormatCtx,argv[1],NULL,NULL)<0)
    {
      fprintf(stderr,"Cannot open input file\n");
      return -1;
    }
  printf("nb_stream is %d\n",pFormatCtx->nb_streams);
  if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
      fprintf(stderr,"Could not find stream information\n");
      return -1;

    }
  pFormatCtx->probesize*=10;
  printf("\033[31mpFormatCtx->probesize is %d\n\033[0m",pFormatCtx->probesize);
  int ret;
  if((ret=av_find_best_stream(pFormatCtx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0))<0)
    {
      fprintf(stderr,"cannot find video stream\n");
      return -1;
    }

  AVStream *st;
  st=pFormatCtx->streams[ret];
  printf("video stream id  is %d\n",ret);
  AVCodecContext *pcodectx;
  pcodectx=st->codec;
  AVCodec *pcodec;
  pcodec=avcodec_find_decoder(pcodectx->codec_id);
 
  if(!pcodec)
    {
      fprintf(stderr,"media type is not supported \n");
      return -1;
    }
  if(pcodec->capabilities&CODEC_CAP_TRUNCATED)
    pcodectx->flags|=CODEC_FLAG_TRUNCATED;

  printf("\033[32mcodec name is %s\n\033[0m",avcodec_get_name(pcodectx->codec_id));

  if((ret=avcodec_open2(pcodectx,pcodec,NULL))<0)
    {
      fprintf(stderr,"failed to open codec\n");
      return -1;
    }




  uint8_t *video_dst_data[4],*video_src_data[4];
  int *video_dst_linesize[4],*video_src_linesize[4];
  ret=av_image_alloc(video_src_data,video_src_linesize,pcodectx->width,pcodectx->height,pcodectx->pix_fmt,1);
  ret=av_image_alloc(video_dst_data,video_dst_linesize,pcodectx->width,pcodectx->height,AV_PIX_FMT_BGR24,1);


  printf("\033[32mpix_fmt's name is %s\n\033[0m",av_get_pix_fmt_name(pcodectx->pix_fmt));
 
  

  SDL_Texture* tex=SDL_CreateTexture(myrender,SDL_PIXELFORMAT_BGR24,SDL_TEXTUREACCESS_STREAMING,pcodectx->width,pcodectx->height);
  //SDL_Texture* tex=SDL_CreateTexture(myrender,SDL_PIXELFORMAT_YV12,SDL_TEXTUREACCESS_STREAMING,pcodectx->width,pcodectx->height);
  if(ret<0)
    {
      fprintf(stderr,"could not allocate ram video buffer\n");
      return -1;
    }
  AVFrame *frame=avcodec_alloc_frame();
  if(!frame)
    {
      fprintf(stderr,"Could not allocate frame\n");
      return -1;
    }
  
  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data=NULL;
  pkt.size=0;
  int getframe=0;
  int n_frame=0;
  SDL_Event e;
  int len;
  struct SwsContext *swsctx;
  swsctx=sws_getContext(pcodectx->width,pcodectx->height,pcodectx->pix_fmt,pcodectx->width,pcodectx->height,AV_PIX_FMT_BGR24,SWS_GAUSS,NULL,NULL,NULL);
  
    while(1)
    {
      ret=SDL_PollEvent(&e);
      
      if(av_read_frame(pFormatCtx,&pkt)>=0 && ret==0)
	{
	  if(pkt.stream_index==1)
	    {
	      if((len=avcodec_decode_video2(pcodectx,frame,&getframe,&pkt))<0)
		{
		  fprintf(stderr,"failed to decoding video\n");
		  return -1;
		}
	      if(getframe )
		{
		  av_image_copy(video_src_data,video_src_linesize,frame->data,frame->linesize,pcodectx->pix_fmt,pcodectx->width,pcodectx->height);
		  sws_scale(swsctx,video_src_data,video_src_linesize,0,pcodectx->height,video_dst_data,video_dst_linesize); 
		  SDL_Rect sdlRT;
		  sdlRT.h = pcodectx->height;
		  sdlRT.w = pcodectx->width;
		  sdlRT.x = 0;
		  sdlRT.y = 0;
		  int iPitch = pcodectx->width*SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_BGR24);
		  //int iPitch = pcodectx->width*SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_YV12);
		  //SDL_UpdateTexture(tex,&sdlRT,video_src_data[0],iPitch);
		  SDL_UpdateTexture(tex,&sdlRT,video_dst_data[0],iPitch);
		  SDL_RenderClear(myrender);
		  SDL_RenderCopy(myrender,tex,NULL,NULL);
		  SDL_RenderPresent(myrender);
		  SDL_Delay(40);

		}
	    }

	}
      else
	{
	  if(myevent(&e)==0)
	    {
	      SDL_Quit();
	      break;
	    }
	}
    }
  return 0;
}




int myevent(SDL_Event* e)
{
  char str[100];
  if(e->type==SDL_KEYDOWN)
    {
      sprintf(str,"%c is pressed\n",e->key.keysym.sym);
      switch(e->key.keysym.sym)
	{
	case SDLK_q:
	  return 0;
	  break;
	default:

	  break;
	
	}
    }
  return 1;
}















