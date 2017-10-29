# DummyFileSystem
DummtFileSystem(DFS) is a VERY Simple , single-folder,on-memory filesystem , with every line commented , for fun and educational purposes . 

<hr>
It supports ... 
<table>
  <tr><td>Operation</td><td>Associated Function</td><td>Exists on file..</td><td>Status</td><td>Version</td><td>Notes</td></tr>
  <tr><td>.create()</td><td>.dfs_iop_create()</td><td>dfs_module.c</td><td>PENDING</td><td>0.0.1</td><td>Only Regular files, Under Develepoment</td></tr>
  <tr><td>.lookup()</td><td>dfs_iop_lookup()</td><td>dfs_module.c</td><td>PENDING</td><td>0.0.1</td><td>Only Regular files, Under Develepoment</td></tr>
</table>

<blockquote><h4>Warning:This filesystem is (yet) very unstable and untested.. , you can test it in a Virtualbox or any VM Technology...</h4></blockquote>

<h3>Lets play with it.. :) </h3>

<table>
  <tr><td>Operation</td><td>Purpose</td></tr>
  <tr><td>cd <PathToFolder>/DummyFileSystem</td><td>Go on code folder...</td></tr>
  <tr><td>make </td><td>Create the linux module </td></tr>
  <tr><td>sudo insmod dfs_module.ko </td><td>load module in the kernel..</td></tr>
  <tr><td>cd test</td><td>go to test folder..</td></tr>
  <tr><td>sudo losetup -f </td><td>lets detect the first available loop device</td></tr>
  <tr><td>sudo losetup /dev/loop<number of the first available loop device..> ./img</td><td>asscosiate the loop device with image file...</td></tr>
  <tr><td>sudo mount -t dfs /dev/loop<loop number> ./mount </td><td>mount filesystem on mount folder...</td></tr>
</table>
