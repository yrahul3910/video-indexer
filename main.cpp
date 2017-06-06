#include "MainWindow.h"
#include <QApplication>
#include <pocketsphinx.h>
#include <stdio.h>
#define MODELDIR "/usr/local/share/pocketsphinx/model"

int main(int argc, char *argv[])
{
    /*if (argc != 2) {
        printf("Usage: main <file>\n");
        return -1;
    }*/

    ps_decoder_t *ps = nullptr;
    cmd_ln_t *config = nullptr;
    FILE *fh;
    char const *hyp;
    int16 buf[512];
    int rv;
    int32 score;
    ps_seg_t *seg;
    const char *segWord;
    int frameRate, startFrame, endFrame;

    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "/en-us/en-us",
                         "-lm", MODELDIR "/en-us/en-us.lm.bin",
                         "-dict", MODELDIR "/en-us/cmudict-en-us.dict",
                         NULL);    
    if (!config) {
        fprintf(stderr, "Failed to create config object, see log for details\n");
        return -1;
    }

    ps = ps_init(config);
    if (!ps) {
        fprintf(stderr, "Failed to create recognizer, see log for details\n");
        return -1;
    }

    char* ffmpegCommand;
    sprintf(ffmpegCommand, "./avconv -i %s -ar 16000 -ac 1 tmp.wav", "videoplayback.mp4");
    system(ffmpegCommand);
    fh = fopen("tmp.wav", "rb");
    if (!fh) {
        fprintf(stderr, "Unable to open file\n");
        return -1;
    }

    // Start decoding of the speech
    // Set to hot word listening mode
    //ps_set_keyphrase(ps, "keyphrase_search", "say the word");
    //ps_set_search(ps, "keyphrase_search");
    rv = ps_start_utt(ps);


    // Read 512 samples at a time and feed them to the decoder
    while (!feof(fh)) {
        size_t nsamp = fread(buf, 2, 512, fh);
        rv = ps_process_raw(ps, buf, nsamp, FALSE, FALSE);
    }

    // Mark end of utterance
    rv = ps_end_utt(ps);

    seg = ps_seg_iter(ps);
    frameRate = cmd_ln_int32_r(config, "-frate");
    printf("Frame rate is %d\n", frameRate);
    while (seg) {
        segWord = ps_seg_word(seg);
        ps_seg_frames(seg, &startFrame, &endFrame);
        // The time computation is not exactly accurate, it's off by a few seconds
        int startTime = (startFrame / frameRate) - 126; // 126 is the length of the file
        int endTime = (endFrame / frameRate) - 126;
        printf("Found %s at time: %d-%d seconds\n\n", segWord, startTime, endTime);
        seg = ps_seg_next(seg);
    }

    // Get recognition result
    hyp = ps_get_hyp(ps, &score);
    printf("Recognized: %s\n", hyp);

    // Release resources
    fclose(fh);
    ps_free(ps);
    cmd_ln_free_r(config);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return 0;
}
