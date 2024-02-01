// Simplest FFmpeg Audio Encoder.cpp : �������̨Ӧ�ó������ڵ㡣
//

/**
* ��򵥵Ļ��� FFmpeg ����Ƶ������
* Simplest FFmpeg Audio Encoder
*
* Դ����
* ������ Lei Xiaohua
* leixiaohua1020@126.com
* �й���ý��ѧ/���ֵ��Ӽ���
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* �޸ģ�
* ���ĳ� Liu Wenchen
* 812288728@qq.com
* ���ӿƼ���ѧ/������Ϣ
* University of Electronic Science and Technology of China / Electronic and Information Science
* https://blog.csdn.net/ProgramNovice
*
* ������ʵ������Ƶ PCM �������ݱ���Ϊѹ��������MP3��WMA��AAC �ȣ���
* ����򵥵� FFmpeg ��Ƶ���뷽��Ľ̡̳�
* ͨ��ѧϰ�����ӿ����˽� FFmpeg �ı������̡�
*
* This software encode PCM data to AAC bitstream.
* It's the simplest audio encoding software based on FFmpeg.
* Suitable for beginner of FFmpeg
*
*/

#include "stdafx.h"

#include <stdio.h>

// �������fopen() ��������ȫ
#pragma warning(disable:4996)

// ��������޷��������ⲿ���� __imp__fprintf���÷����ں��� _ShowError �б�����
#pragma comment(lib, "legacy_stdio_definitions.lib")
extern "C"
{
	// ��������޷��������ⲿ���� __imp____iob_func���÷����ں��� _ShowError �б�����
	FILE __iob_func[3] = { *stdin, *stdout, *stderr };
}

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
// Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
#else
// Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
};
#endif
#endif


int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
	int ret;
	int got_frame;
	AVPacket enc_pkt;

	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;
	while (1)
	{
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame)
		{
			ret = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d.\n", enc_pkt.size);
		// mux encoded frame
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
		{
			break;
		}
	}
	return ret;
}

int main(int argc, char* argv[])
{
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* audio_stream;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;

	uint8_t* frame_buf;
	AVFrame* pFrame;
	AVPacket pkt;

	int got_frame = 0;
	int size = 0;
	int ret = 0;

	FILE *fp_in = fopen("tdjm.pcm", "rb"); // ���� PCM �ļ�
	int framenum = 1000; // Audio frame number
	const char* out_file = "tdjm.aac"; // ��� URL
	int i;

	av_register_all();

	// Method 1
	// pFormatCtx = avformat_alloc_context();
	// fmt = av_guess_format(NULL, out_file, NULL);
	// pFormatCtx->oformat = fmt;

	// Method 2 (More simple)
	avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	fmt = pFormatCtx->oformat;

	// Open Output URL
	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
	{
		printf("Can't open output file.\n");
		return -1;
	}

	audio_stream = avformat_new_stream(pFormatCtx, 0);
	if (audio_stream == NULL)
	{
		printf("Can't create audio stream.\n");
		return -1;
	}

	pCodecCtx = audio_stream->codec;
	pCodecCtx->codec_id = fmt->audio_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->sample_rate = 44100;
	pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	pCodecCtx->bit_rate = 64000;

	//pCodec = avcodec_find_encoder(fmt->audio_codec);
	//if (!pCodec)
	//{
	//	printf("Can't find encoder.\n");
	//	return -1;
	//}
	//pCodecCtx = audio_stream->codec;
	//pCodecCtx->codec_id = fmt->audio_codec;
	//pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	//pCodecCtx->sample_fmt = pCodec->sample_fmts[0];
	//pCodecCtx->sample_rate = 44100;
	//pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	//pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	//pCodecCtx->bit_rate = 64000;
	//pCodecCtx->profile = FF_PROFILE_AAC_MAIN;
	//pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

	// Print some information
	av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec)
	{
		printf("Can not find encoder!\n");
		return -1;
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("Failed to open encoder.\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrame->nb_samples = pCodecCtx->frame_size;
	pFrame->format = pCodecCtx->sample_fmt;

	// ���㻺�������С
	size = av_samples_get_buffer_size(NULL, pCodecCtx->channels, pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);
	// ���仺��
	frame_buf = (uint8_t *)av_malloc(size);
	if (!frame_buf)
	{
		printf("Can't malloc frame buffer.\n");
		return -1;
	}
	avcodec_fill_audio_frame(pFrame, pCodecCtx->channels, pCodecCtx->sample_fmt,
		(const uint8_t*)frame_buf, size, 1);

	// Write Header
	avformat_write_header(pFormatCtx, NULL);

	av_new_packet(&pkt, size);

	for (i = 0; i < framenum; i++)
	{
		// Read PCM
		if (fread(frame_buf, sizeof(char), size, fp_in) <= 0)
		{
			printf("Failed to read raw data! \n");
			return -1;
		}
		else if (feof(fp_in))
		{
			break;
		}
		pFrame->data[0] = frame_buf;  // PCM Data

		pFrame->pts = i * 100;
		got_frame = 0;
		// Encode
		ret = avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &got_frame);
		if (ret < 0)
		{
			printf("Failed to encode!\n");
			return -1;
		}
		if (got_frame == 1)
		{
			printf("Succeed to encode 1 frame! \tsize:%5d\n", pkt.size);
			pkt.stream_index = audio_stream->index;
			ret = av_write_frame(pFormatCtx, &pkt);
			av_free_packet(&pkt);
		}
	}

	// Flush Encoder
	ret = flush_encoder(pFormatCtx, 0);
	if (ret < 0)
	{
		printf("Flushing encoder failed.\n");
		return -1;
	}


	// Write Trailer
	av_write_trailer(pFormatCtx);

	printf("Encode Successful.\n");

	// Clean
	if (audio_stream)
	{
		avcodec_close(audio_stream->codec);
		av_free(pFrame);
		av_free(frame_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(fp_in);
	
	return 0;
}