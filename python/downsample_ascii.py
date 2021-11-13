import os
import numpy as np
import open3d as o3d
# Open file path
files = os.listdir("/home/mcnl/mcnl/dataset/8iVSLF/Thaidancer_viewdep/Ply")

comp_rate = input("Compression Rate:")

pointcloud = o3d.geometry.PointCloud()
to_reset = True
files.sort()

for f in files:
    pcd = o3d.io.read_point_cloud("./std_clr/"+f)
    print(pcd)
    downpcd = pcd.voxel_down_sample(voxel_size=1/float(comp_rate))
    print("down:")
    print(downpcd)
    o3d.io.write_point_cloud("./downsample/test/downsampled_"+f, downpcd, write_ascii=True)
