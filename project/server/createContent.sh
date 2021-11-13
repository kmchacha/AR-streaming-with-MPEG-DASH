#! /bin/bash
# Date : '21.10.25
# Filename : server.sh
# Writer : Kim Nak-hyeon


# TMC2 DIRECTORY
TMC2_DIR=".."

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

### make dir for files will be created ###
mkdir $STREAM_PATH/$CONTENTS_NAME

mkdir $STREAM_PATH/$CONTENTS_NAME/high
mkdir $STREAM_PATH/$CONTENTS_NAME/mid
mkdir $STREAM_PATH/$CONTENTS_NAME/low

touch "$CONTENTS_NAME.log"
chmod 644 "$CONTENTS_NAME.log"

#####################################
### Frame Sampling for adjust FPS ###

mkdir $DATA_PATH/${FPS}fr

ORIGIN_N_FRAMES=$(ls $DATA_PATH/*.ply | wc -l)

target_d="$DATA_PATH/${FPS}fr"
sampling_term=$((30 / $FPS))

idx=$START_FRAME
echo smpling term: $sampling_term
for((f_idx = $START_FRAME; f_idx < $((START_FRAME + ORIGIN_N_FRAMES)); f_idx += $sampling_term ))
do
	target_f="*$f_idx*.ply"

	cp $DATA_PATH/$target_f $target_d
	mv $target_d/$target_f $target_d/${idx}.ply 2> /dev/null
	if [ $? -ne 0 ]
	then
		echo "Sampling MSG: Already Sampled Frame exists or Somthing wrong"
		rm -rf ls $DATA_PATH/$target_d/!\([0-9][0-9][0-9][0-9].ply\)
		break
	fi
	
	idx=$(($idx + 1))
done

#####################################
### Encode ###

NUM_OF_FRAMES=$(ls $target_d | wc -l)
NUM_OF_SEG=$((NUM_OF_FRAMES / $((FPS * SEG_TS))))


echo $NUM_OF_SEG
FRAME_COUNT=$((NUM_OF_FRAMES / NUM_OF_SEG))

LOW_OUT=""
MID_OUT=""
HIGH_OUT=""

HIGH_BANDWIDTH=0
MID_BANDWIDTH=0
LOW_BANDWIDTH=0

for((id = 0; id < $NUM_OF_SEG; id++))
do
$TMC2_DIR/bin/PccAppEncoder \
--configurationFolder=$TMC2_DIR/cfg/ \
--config=$TMC2_DIR/cfg/common/ctc-common.cfg \
--config=$TMC2_DIR/cfg/condition/ctc-all-intra.cfg \
--config=$CFG_PATH \
--config=$TMC2_DIR/cfg/rate/low.cfg \
--frameCount="$FRAME_COUNT" \
--startFrameNumber="$START_FRAME" \
--resolution="$RESOLUTION" \
--uncompressedDataPath=$target_d/%04d.ply \
--compressedStreamPath=$STREAM_PATH/$CONTENTS_NAME/low/low_s$id.bin

$TMC2_DIR/bin/PccAppEncoder \
--configurationFolder=$TMC2_DIR/cfg/ \
--config=$TMC2_DIR/cfg/common/ctc-common.cfg \
--config=$TMC2_DIR/cfg/condition/ctc-all-intra.cfg \
--config=$CFG_PATH \
--config=$TMC2_DIR/cfg/rate/mid.cfg \
--frameCount="$FRAME_COUNT" \
--startFrameNumber="$START_FRAME" \
--resolution="$RESOLUTION" \
--uncompressedDataPath=$target_d/%04d.ply \
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
--uncompressedDataPath=$target_d/%04d.ply \
--compressedStreamPath=$STREAM_PATH/$CONTENTS_NAME/high/high_s$id.bin


### calc minimum bandwidth ###
LOW_OUT=$STREAM_PATH/$CONTENTS_NAME/low/low_s$id.bin
MID_OUT=$STREAM_PATH/$CONTENTS_NAME/mid/mid_s$id.bin
HIGH_OUT=$STREAM_PATH/$CONTENTS_NAME/high/high_s$id.bin

HIGH_TEMP=$(du -sk $HIGH_OUT | cut -f1)
MID_TEMP=$(du -sk $MID_OUT | cut -f1)
LOW_TEMP=$(du -sk $LOW_OUT | cut -f1)

if [ $HIGH_TEMP -gt $HIGH_BANDWIDTH ]
then
	HIGH_BANDWIDTH=$HIGH_TEMP
fi

if [ $MID_TEMP -gt $MID_BANDWIDTH ]
then
	MID_BANDWIDTH=$MID_TEMP
fi

if [ $LOW_TEMP -gt $LOW_BANDWIDTH ]
then
	LOW_BANDWIDTH=$LOW_TEMP
fi

echo "==========$id-th Frame was created==========" >> "$CONTENTS_NAME.log"
echo "Size(kb) | OUTPUT" >> "$CONTENTS_NAME.log"
du -sk $LOW_OUT $MID_OUT $HIGH_OUT >> "$CONTENTS_NAME.log"

echo $LOW_BANDWIDTH $MID_BANDWIDTH $HIGH_BANDWIDTH >> $STREAM_PATH/$CONTENTS_NAME/bandwidth.txt

START_FRAME=$((START_FRAME + FRAME_COUNT))

done


#### Generate MPD ####
dur_H=$(($(($NUM_OF_SEG * $SEG_TS))/3600))
dur_M=$(($(($(($NUM_OF_SEG * $SEG_TS)) - $dur_H * 3600))/60))
dur_S=$(($(($NUM_OF_SEG * $SEG_TS)) % 60))
minBufferTime=$SEG_TS

presentation_dur="PT${dur_H}H${dur_M}M${dur_S}.000S"
baseURL="http://203.252.121.219/video/$CONTENTS_NAME/"

HIGH_BANDWIDTH=$(($HIGH_BANDWIDTH*1000))
MID_BANDWIDTH=$(($MID_BANDWIDTH*1000))
LOW_BANDWIDTH=$(($LOW_BANDWIDTH*1000))

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "<MPD xmlns=\"urn:mpeg:dash:schema:mpd:2011\" minBufferTime=\"PTS0H0M$SEG_TS.000\" type=\"static\" mediaPresentationDuration=\"$presentation_dur\" maxSegmentDuration=\"PT0H0M$SEG_TS.000S\" profiles=\"urn:mpeg:dash:profile:full:2011\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "  <BaseURL>$baseURL</BaseURL>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "    <Priod duration=\"$presentation_dur\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "      <AdaptationSet sgmentAlignment=\"true\" maxWidth=\"1024\" maxHeight=\"1024\" maxFrameRate=\"30\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

## HIGH ##
echo "    	  <Representation id=\"0\" mimeType=\"video/mp4\" codecs=\"avc1.d44020\" width=\"1024\" height=\"1024\" framRate=\"$FPS\" sar=\"1:1\" startWithSAP=\"1\" bandwidth=\"$HIGH_BANDWIDTH\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			<SegmentBase>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

echo "			  <Initialization sourceURL=\"high/${CONTENTS_NAME}_high_init.mp4\"/>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			</SegmentBase>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			<SegmentList duration=\"$SEG_TS\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

for (( id = 0; id < $NUM_OF_SEG; id++ ))
do
	echo "			  <SegmentURL media=\"high/high_s$id.bin\"/>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
done

echo "			</SegmentList>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "		  </Representation>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

## MID ##
echo "    	  <Representation id=\"1\" mimeType=\"video/mp4\" codecs=\"avc1.d44020\" width=\"1024\" height=\"1024\" framRate=\"$FPS\" sar=\"1:1\" startWithSAP=\"1\" bandwidth=\"$MID_BANDWIDTH\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			<SegmentBase>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			  <Initialization sourceURL=\"mid/${CONTENTS_NAME}_mid_init.mp4\"/>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			</SegmentBase>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			<SegmentList duration=\"$SEG_TS\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

for (( id = 0; id < $NUM_OF_SEG; id++ ))
do
	echo "			  <SegmentURL media=\"mid/mid_s$id.bin\"/>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
done

echo "			</SegmentList>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "		  </Representation>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

## LOW ##
echo "    	  <Representation id=\"2\" mimeType=\"video/mp4\" codecs=\"avc1.d44020\" width=\"1024\" height=\"1024\" framRate=\"$FPS\" sar=\"1:1\" startWithSAP=\"1\" bandwidth=\"$LOW_BANDWIDTH\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			<SegmentBase>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			  <Initialization sourceURL=\"low/${CONTENTS_NAME}_low_init.mp4\"/>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			</SegmentBase>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "			<SegmentList duration=\"$SEG_TS\">" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

for (( id = 0; id < $NUM_OF_SEG; id++ ))
do
	echo "			  <SegmentURL media=\"low/low_s$id.bin\"/>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
done

echo "			</SegmentList>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "		  </Representation>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd

echo "		</AdaptationSet>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "    </Period>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
echo "</MPD>" >> $STREAM_PATH/$CONTENTS_NAME/$CONTENTS_NAME.mpd
