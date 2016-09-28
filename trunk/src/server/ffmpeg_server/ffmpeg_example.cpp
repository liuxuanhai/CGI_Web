#include <vedioRemux.h>
#include <vedioTranscode.h>
#include <audioTranscode.h>
#include <allTranscode.h>

using namespace std;

int main()
{
	//vedioRemux("test.MOV", "vedioRemux.mp4");
	//vedioTranscode("test.MOV", "vedioTranscode.mp4");
	//audioTranscode("yaya.mp3", "audioTranscode.aac");
	//allTranscode("yaya.mp3", "allTranscode.ogg");
	//allTranscode("yaya.mp3", "allTranscode.aac");
	//allTranscode("test.MOV", "allTranscode.mp4");
	allTranscode("5e99757da60d462e", "allTranscode.mp3");
	return 0;
}
