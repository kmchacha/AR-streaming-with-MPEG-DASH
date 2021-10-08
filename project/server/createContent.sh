#! /bin/bash
# Date : '21.07.15
# Filename : server.sh
# Writer : Kim Nak-hyeon

# TMC2 DIRECTORY
TMC2_DIR="/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2"

# Compressed Stream Path
STREAM_PATH="/var/www/html/video"

# segment timeslice 1 sec
SEG_TS=1

### check the parameter ##
if [ $# -ne 6 ]; then
	echo "script need 5 parameter"
	echo "Usage: ./createContent.sh [DATA_PATH] [CFG_PATH] [START_FRAME] [FPS] [resolution] [CONENTS_NAME]"

	exit 1
fi

DATA_PATH=$1
CFG_PATH=$2
START_FRAME=$3
FPS=$4
RESOLUTION=$5
CONTENTS_NAME=$6

### make dir for new files ###
cd $STREAM_PATH
mkdir $CONTENTS_NAME

cd $STREAM_PATH/$CONTENTS_NAME
mkdir high
mkdir mid
mkdir low

touch ""$CONTENTS_NAME".log"
chmod 644 ""$CONTENTS_NAME".log"
###############################

NUM_OF_FRAMES=$(ls $1 | wc -l)
NUM_OF_SEG=$((NUM_OF_FRAMES / $((FPS * SEG_TS))))

echo $NUM_OF_SEG
FRAME_COUNT=$((NUM_OF_FRAMES / NUM_OF_SEG))

LOW_OUT=""
MID_OUT=""
HIGH_OUT=""

for((id = 0; id < $NUM_OF_SEG; id++))
do
$TMC2_DIR/bin/PccAppEncoder \
--configurationFolder=$TMC2_DIR/cfg/ \
--config=$TMC2_DIR/cfg/common/ctc-common.cfg \
--config=$TMC2_DIR/cfg/condition/ctc-all-intra.cfg \
--config=$CFG_PATH \
--config=$TMC2_DIR/cfg/rate/low.cfg \
--frameCount="$FRAME_COUNT" \
--startFrameNumber="$START_FRAME"\
--resolution="$RESOLUTION" \
--compressedStreamPath=$STREAM_PATH/$CONTENTS_NAME/low/low_s$id.bin

$TMC2_DIR/bin/PccAppEncoder \
--configurationFolder=$TMC2_DIR/cfg/ \
--config=$TMC2_DIR/cfg/common/ctc-common.cfg \
--config=$TMC2_DIR/cfg/condition/ctc-all-intra.cfg \
--config=$CFG_PATH \
--config=$TMC2_DIR/cfg/rate/mid.cfg \
--frameCount="$FRAME_COUNT" \
--startFrameNumber="$START_FRAME"\
--resolution="$RESOLUTION" \
--compressedStreamPath=$STREAM_PATH/$CONTENTS_NAME/mid/mid_s$id.bin

$TMC2_DIR/bin/PccAppEncoder \
--configurationFolder=$TMC2_DIR/cfg/ \
--config=$TMC2_DIR/cfg/common/ctc-common.cfg \
--config=$TMC2_DIR/cfg/condition/ctc-all-intra.cfg \
--config=$CFG_PATH \
--config=$TMC2_DIR/cfg/rate/high.cfg \
--frameCount="$FRAME_COUNT" \
--startFrameNumber="$START_FRAME" \
--resolution="$RESOLUTION" \
--compressedStreamPath=$STREAM_PATH/$CONTENTS_NAME/high/high_s$id.bin


LOW_OUT="$STREAM_PATH/$CONTENTS_NAME/low/low_s$id.bin"
MID_OUT="$STREAM_PATH/$CONTENTS_NAME/mid/mid_s$id.bin"
HIGH_OUT="$STREAM_PATH/$CONTENTS_NAME/high/high_s$id.bin"

echo "==========$id-th Frame was created==========" >> ""$CONTENTS_NAME".log"
echo "Size(kb) | OUTPUT" >> ""$CONTENTS_NAME".log"
du -sk $LOW_OUT $MID_OUT $HIGH_OUT >> ""$CONTENTS_NAME".log"

START_FRAME=$((START_FRAME + FRAME_COUNT))

done
