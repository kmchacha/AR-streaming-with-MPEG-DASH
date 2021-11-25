# AR Point Cloud Streaming with MPEG-DASH

This project is for AR point cloud streaming based on MPEG-DASH.

We used [libdash](http://), [V-PCC tmc-2](http://) and [Open3D](https://github.com/intel-isl/Open3D).


## Step 1: Build

On Ubuntu/macOS:

```bash
git clone https://github.com/rudals0215/AR-streaming-with-MPEG-DASH.git
cd AR-streaming-with-MPEG-DASH
mkdir build
cd build
cmake ..
make
```


## Step 2-1: Execute - Client

On Ubuntu/macOS:
```bash
cd build/bin

./Main [PORT]
```

Note: (Optional) If you want to know timeLog.

```bash
cd build/bin
mkdir timeLog
```

Note: Whenever you execute this program, you have to remove the .ply file in dec_test.
```bash
cd dec_test

```

## Step 2-2: Execute - Server

On Ubuntu/macOS:
```bash
cd server

sudo ./createContent [DATA_PATH] [CFG_PATH] [START_FRAME] [FPS] [resolution] [CONTENTS_NAME]
```

DATA_PATH : Point cloud data consisting contents.

CFG_PATH : Written sequence configuration (sample : /cfg/sequence/loot_vox10.cfg)

STRAT_FRAME : Starting Index of Point cloud data consisting contents

CONTENTS_NAME : Contents name that one want to create

