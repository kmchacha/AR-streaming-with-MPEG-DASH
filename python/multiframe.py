import os
import numpy as np
import open3d as o3d
# Open file path

files = os.listdir("./downsample/mixed")

vis = o3d.visualization.Visualizer()
# Create a playback window
vis.create_window()
pointcloud = o3d.geometry.PointCloud()
to_reset = True
vis.add_geometry(pointcloud)

files.sort()

for f in files:
    pcd = o3d.io.read_point_cloud("./downsample/low/"+f, format="xyzrgb")
    vtx = np.asarray(pcd.points).reshape((-1,3))
    clrs = np.asarray(pcd.colors).reshape((-1,3))
    clrs = clrs / 256
    # print(clrs)
    
    pointcloud.points = o3d.utility.Vector3dVector(vtx)
    pointcloud.colors = o3d.utility.Vector3dVector(clrs)
    vis.update_geometry(pointcloud)
    
    if to_reset:
        vis.reset_view_point(True)
        to_reset = False

    vis.poll_events()
    vis.update_renderer()

for f in files:
    pcd = o3d.io.read_point_cloud("./downsample/mid/"+f, format="xyzrgb")
    vtx = np.asarray(pcd.points).reshape((-1,3))
    clrs = np.asarray(pcd.colors).reshape((-1,3))
    clrs = clrs / 256
    # print(clrs)

    pointcloud.points = o3d.utility.Vector3dVector(vtx)
    pointcloud.colors = o3d.utility.Vector3dVector(clrs)
    vis.update_geometry(pointcloud)

    if to_reset:
        vis.reset_view_point(True)
        to_reset = False

    vis.poll_events()
    vis.update_renderer()

for f in files:
    pcd = o3d.io.read_point_cloud("./downsample/high/"+f, format="xyzrgb")
    vtx = np.asarray(pcd.points).reshape((-1,3))
    clrs = np.asarray(pcd.colors).reshape((-1,3))
    clrs = clrs / 256
    # print(clrs)

    pointcloud.points = o3d.utility.Vector3dVector(vtx)
    pointcloud.colors = o3d.utility.Vector3dVector(clrs)
    vis.update_geometry(pointcloud)

    if to_reset:
        vis.reset_view_point(True)
        to_reset = False

    vis.poll_events()
    vis.update_renderer()

