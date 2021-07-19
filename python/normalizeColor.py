import os
import numpy as np
import open3d as o3d
# Open file path
files = os.listdir("/home/mcnl/mcnl/dataset/8iVSLF/Thaidancer_viewdep/Ply")

pointcloud = o3d.geometry.PointCloud()

files.sort()

for f in files:
    pcd = o3d.io.read_point_cloud("/home/mcnl/mcnl/dataset/8iVSLF/Thaidancer_viewdep/Ply/"+f, format='xyzrgb')
    vtx = np.asarray(pcd.points).reshape((-1,3))
    clrs = np.asarray(pcd.colors).reshape((-1,3))
    clrs = clrs / 256
    print(clrs)
    
    pointcloud.points = o3d.utility.Vector3dVector(vtx)
    pointcloud.colors = o3d.utility.Vector3dVector(clrs)

    print(pointcloud.colors[0])

    o3d.io.write_point_cloud("./std_clr/"+f, pointcloud, write_ascii=True)
