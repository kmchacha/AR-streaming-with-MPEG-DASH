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


## Step 2: Execute

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
rmdir ply*
```
