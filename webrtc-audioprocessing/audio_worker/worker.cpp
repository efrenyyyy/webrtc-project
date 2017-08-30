#include "webrtc/modules/audio_processing/include/audio_processing.h"
#include "webrtc/modules/include/module_common_types.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "webrtc/common_audio/resampler/include/resampler.h"

void InitAudioProcessing(webrtc::AudioProcessing *apm)
{
	apm->level_estimator()->Enable(true);

	//apm->echo_cancellation()->Enable(true);

	//apm->echo_cancellation()->enable_metrics(true);

	//apm->echo_cancellation()->enable_drift_compensation(true); //为TRUE时必须调用set_stream_drift_samples

	apm->gain_control()->Enable(true);

	apm->high_pass_filter()->Enable(true);

	apm->noise_suppression()->set_level(webrtc::NoiseSuppression::kHigh);

	apm->noise_suppression()->Enable(true);

	apm->voice_detection()->Enable(true);

	apm->voice_detection()->set_likelihood(webrtc::VoiceDetection::kModerateLikelihood);

	apm->Initialize();

}
#define MAX_BUFFER_SIZE 480
void main(int argc, char **argv)
{
	int32_t size, ret, length = 0;
	int64_t beg, end;
	int16_t tmp[MAX_BUFFER_SIZE * 2];
	int16_t buffer[2][MAX_BUFFER_SIZE];
	int16_t buffer2[2][MAX_BUFFER_SIZE];
	int16_t *src[2] = { buffer[0], buffer[1] };
	int16_t *dst[2] = { buffer2[0], buffer2[1] };
	FILE *ifp, *ofp;
	webrtc::AudioFrame frame;
	webrtc::SimulatedClock clock(0);
	char *input;
	webrtc::AudioProcessing	*apm = webrtc::AudioProcessing::Create();

	input = argv[1];
	ifp = fopen(input, "rb");
	fseek(ifp, 0, SEEK_END);
	size = ftell(ifp);
	fseek(ifp, 0, SEEK_SET);

	ofp = fopen("webrtc.pcm", "wb");

	InitAudioProcessing(apm);

	while (1)
	{
		ret = fread(tmp, sizeof(int16_t), MAX_BUFFER_SIZE*2, ifp);
		if (ret < 0)
		{
			printf("error %d\n", ftell(ifp));
			break;
		}
		length += MAX_BUFFER_SIZE*4;
		//apm->ProcessReverseStream();
		beg = clock.CurrentNtpInMilliseconds();
		frame.UpdateFrame(0, 0, tmp, 480, 48000, webrtc::AudioFrame::kNormalSpeech, webrtc::AudioFrame::kVadActive, 2);
		ret = apm->ProcessStream(&frame);
		end = clock.CurrentNtpInMilliseconds();
		if (end - beg > 10)
		{
			printf("beg:%lld, end:%lld\n", beg, end);
		}
		fwrite(frame.data(), 1, MAX_BUFFER_SIZE*2*sizeof(uint16_t), ofp);
		if (size <= length)
			break;
	}
	//audiosdk.release();
	fclose(ofp);
	if (apm)
		delete apm;
}