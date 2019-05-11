## If you want to add a new applet to busybox and make patch for it, you are advised to read this.

1. The ../docs/new-applet-HOWTO.txt can help you to add a new applet to busybox, just notice the entry order you added in ../include/applets.h.

2. You are advised to use git format-patch to make a patch file for the new applet, then move the patch file in this document and remember to modify the files' path in the patch file.

3. If your new applet should be based on previous applets added in this document, your patch file should be based on previous patch files also!

4. Notice to make the new applet "default n" in ../***/Config.in .

5. When you patched these file ,you need to "make menuconfig" to choose the
new applet.
