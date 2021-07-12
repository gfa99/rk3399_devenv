/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "RTNodeVFilterEptzDemo.h"          // NOLINT
#include "rockit/RTNodeCommon.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RTNodeVFilterEptz"
#define kStubRockitEPTZDemo                MKTAG('e', 'p', 'd', 'm')

RTNodeVFilterEptz::RTNodeVFilterEptz() {
}

RTNodeVFilterEptz::~RTNodeVFilterEptz() {
}

RT_RET RTNodeVFilterEptz::open(RTTaskNodeContext *context) {
    RtMetaData* inputMeta   = context->options();
    RT_RET err              = RT_OK;

    RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_WIDTH, &mSrcWidth));
    RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_HEIGHT, &mSrcHeight));
    RT_ASSERT(inputMeta->findInt32(OPT_EPTZ_CLIP_WIDTH, &mClipWidth));
    RT_ASSERT(inputMeta->findInt32(OPT_EPTZ_CLIP_HEIGHT, &mClipHeight));

    mRoiRegion.x = 0;
    mRoiRegion.y = 0;
    mRoiRegion.w = mSrcWidth;
    mRoiRegion.h = mSrcHeight;

    mEptzInfo.eptz_src_width = mSrcWidth;
    mEptzInfo.eptz_src_height = mSrcHeight;
    mEptzInfo.eptz_dst_width = mSrcWidth;
    mEptzInfo.eptz_dst_height = mSrcHeight;
    mEptzInfo.camera_dst_width = mClipWidth;
    mEptzInfo.camera_dst_height = mClipHeight;
    //2K以上sensor建议使用1280x720数据,低于2K使用640x360
    mEptzInfo.eptz_npu_width = 640;
    mEptzInfo.eptz_npu_height = 360;
    //V2远距离建议0.4，V3近距离建议0.6
    mEptzInfo.eptz_facedetect_score_shold = 0.40;
    mEptzInfo.eptz_zoom_speed = 1;
    mEptzInfo.eptz_fast_move_frame_judge = 5;
    mEptzInfo.eptz_zoom_frame_judge = 10;
    mEptzInfo.eptz_threshold_x = 80;
    mEptzInfo.eptz_threshold_y = 45;
    if (mEptzInfo.camera_dst_width >= 1920) {
        mEptzInfo.eptz_iterate_x = 6;
        mEptzInfo.eptz_iterate_y = 3;
    } else if (mEptzInfo.camera_dst_width >= 1280) {
        mEptzInfo.eptz_iterate_x = 6;
        mEptzInfo.eptz_iterate_y = 3;
    } else {
        mEptzInfo.eptz_iterate_x = 4;
        mEptzInfo.eptz_iterate_y = 2;
    }
    RT_LOGD("eptz_info src_wh [%d %d] dst_wh[%d %d], threshold_xy[%d %d] "
           "iterate_xy[%d %d] ratio[%.2f] \n",
           mEptzInfo.eptz_src_width, mEptzInfo.eptz_src_height,
           mEptzInfo.eptz_dst_width, mEptzInfo.eptz_dst_height, mEptzInfo.eptz_threshold_x,
           mEptzInfo.eptz_threshold_y, mEptzInfo.eptz_iterate_x, mEptzInfo.eptz_iterate_y, mClipRatio);

    mLastXY[0] = 0;
    mLastXY[1] = 0;
    mLastXY[2] = mEptzInfo.eptz_dst_width;
    mLastXY[3] = mEptzInfo.eptz_dst_height;
    eptzConfigInit(&mEptzInfo);

    return RT_OK;
}

RT_RET RTNodeVFilterEptz::process(RTTaskNodeContext *context) {
    RT_RET         err       = RT_OK;
    RTMediaBuffer *srcBuffer = RT_NULL;
    RTMediaBuffer *dstBuffer = RT_NULL;

    if (!access("/tmp/eptz_mode1", 0)){
        system("rm /tmp/eptz_mode1");
        mEptzInfo.eptz_fast_move_frame_judge = 5;
        mEptzInfo.eptz_zoom_frame_judge = 10;
        setEptzMode(1);
    }

    if (!access("/tmp/eptz_mode2", 0)){
        system("rm /tmp/eptz_mode2");
        mEptzInfo.eptz_fast_move_frame_judge = 10;
        mEptzInfo.eptz_zoom_frame_judge = 15;
        setEptzMode(2);
    }

    // 此处是上级NN人脸检测节点输出人脸区域信息，SDK默认数据流路径是scale1->NN->EPTZ
    if (context->hasInputStream("image:rect")) {
        INT32 count = context->inputQueueSize("image:rect");
        if(count == 0){
            EptzAiData eptz_ai_data;
            eptz_ai_data.face_count = 0;
            calculateClipRect(&eptz_ai_data, mLastXY, true, 5);
        }
        while (count) {
            dstBuffer = context->dequeInputBuffer("image:rect");
            if (dstBuffer == RT_NULL)
                continue;

            count--;

            void* result = getAIDetectResults(dstBuffer);
            RTRknnAnalysisResults *nnResult  = reinterpret_cast<RTRknnAnalysisResults *>(result);
            if (nnResult != RT_NULL) {
                RTRect result;
                INT32 faceCount = nnResult->counter;
                RT_RET ret = RT_OK;
                EptzAiData eptz_ai_data;
                eptz_ai_data.face_data = (FaceData *)malloc(faceCount * sizeof(FaceData));
                eptz_ai_data.face_count = faceCount;
                if(eptz_ai_data.face_data){
                  for(int i = 0; i<faceCount; i++){
                      eptz_ai_data.face_data[i].left = nnResult->results[i].face_info.object.box.left;
                      eptz_ai_data.face_data[i].top = nnResult->results[i].face_info.object.box.top;
                      eptz_ai_data.face_data[i].right = nnResult->results[i].face_info.object.box.right;
                      eptz_ai_data.face_data[i].bottom = nnResult->results[i].face_info.object.box.bottom;
                      eptz_ai_data.face_data[i].score = nnResult->results[i].face_info.object.score;
                  }
                }
                calculateClipRect(&eptz_ai_data, mLastXY);
                if(eptz_ai_data.face_data)
                  free(eptz_ai_data.face_data);
            }
            dstBuffer->release();
        }
    } else {
        RT_LOGE("don't has nn data stream!");
    }

    // 此处是用于预览的原始YUV数据，SDK默认数据流路径是bypass->EPTZ->RGA(输出给下级节点裁剪)
    INT32 count = context->inputQueueSize("image:nv12");
    while (count) {
        srcBuffer = context->dequeInputBuffer("image:nv12");
        if (srcBuffer == RT_NULL)
            continue;
        count--;
        INT32 streamId = context->getOutputInfo()->streamId();
        dstBuffer = srcBuffer;
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_RECT_X, (INT32)mLastXY[0] % 2 != 0 ? (INT32)mLastXY[0] - 1 : (INT32)mLastXY[0]);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_RECT_Y, (INT32)mLastXY[1] % 2 != 0 ? (INT32)mLastXY[1] - 1 : (INT32)mLastXY[1]);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_RECT_W, (INT32)mLastXY[2] % 2 != 0 ? (INT32)mLastXY[2] - 1 : (INT32)mLastXY[2]);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_RECT_H, (INT32)mLastXY[3] % 2 != 0 ? (INT32)mLastXY[3] - 1 : (INT32)mLastXY[3]);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_VIR_WIDTH, mSrcWidth);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_VIR_HEIGHT, mSrcHeight);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_DST_RECT_X, 0);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_DST_RECT_Y, 0);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_DST_RECT_W, mClipWidth);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_DST_RECT_H, mClipHeight);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_DST_VIR_WIDTH, mClipWidth);
        dstBuffer->extraMeta(streamId)->setInt32(OPT_FILTER_DST_VIR_HEIGHT, mClipHeight);
        context->queueOutputBuffer(dstBuffer);
    }
    return err;
}

RT_RET RTNodeVFilterEptz::close(RTTaskNodeContext *context) {
    RT_RET err = RT_OK;

    return err;
}

static RTTaskNode* createEptzFilter() {
    return new RTNodeVFilterEptz();
}

/*****************************************
 * register node stub to RTTaskNodeFactory
 *****************************************/
RTNodeStub node_stub_filter_eptz_demo {
    .mUid          = kStubRockitEPTZDemo,
    .mName         = "rkeptz",
    .mVersion      = "v1.0",
    .mCreateObj    = createEptzFilter,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC,  {RT_NULL, RT_NULL} },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_filter_eptz_demo);

