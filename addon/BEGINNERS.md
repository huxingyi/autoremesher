# First test in Blender, for someone who has never used it

For Blender 4.5 LTS on Ubuntu, on a laptop with a touchpad. Statements about
what the add-on does are checked against its code (`autoremesher_blender/`).
Statements about Blender's own interface are from Blender 4.x and marked
*(not tried here)* where they could not be checked, because Blender was not
installed when this was written.

## 1. Install Blender 4.5 LTS

```
sudo snap install blender --channel=4.5lts/stable --classic
```

`snap info blender` (2026-10-15) lists `4.5lts/stable` as 4.5.14, classic
confinement. Classic means Blender can start the remesher from your home
folder; the add-on also drops the snap's library paths before it starts the
remesher, so the remesher uses the system Qt it was built with.

Blender 5.x is not for this laptop: it needs Intel Kaby Lake or newer graphics,
the HD 5500 is older (https://www.blender.org/download/requirements/).

## 2. Build the add-on zip and the remesher

```
sh ~/Documents/Autoremesher/autoremesher-addon/addon/package.sh
```

It prints two paths. Keep the terminal open; you need both in step 4.

## 3. Start Blender from a terminal

```
blender
```

Starting it from a terminal shows the remesher's messages there if something
goes wrong. A splash screen appears; click anywhere outside it to close it.
You now see the 3D Viewport (the big area) with a cube, a camera and a light,
the Outliner (list of objects, top right) and the Properties editor (bottom
right, a column of small icon tabs on its left edge).

## 4. Install the add-on (once)

1. Menu **Edit > Preferences**. A Preferences window opens.
2. Click **Get Extensions** in its left column.
3. At the top right of that page, click the small **down arrow** and choose
   **Install from Disk...** Pick the zip from step 2
   (`build/addon/quadami-0.1.0.zip`). *(not tried here)*
4. Click **Add-ons** in the left column, find **Quadami** in the list and
   click the arrow left of its name to expand it.
5. In **Remesher executable**, click the folder icon and pick the remesher
   from step 2 (`build/app/autoremesher`), or paste the path. The line under
   it must say **Using: ...** with a check mark; "not found" means the path is
   wrong.
6. Close the Preferences window. Preferences save automatically.

## 5. Moving around without a mouse

The easiest way on a touchpad is the gizmo at the top right of the 3D
Viewport:

- **Orbit:** click and drag on the coloured axis ball (X, Y, Z circles).
- **Zoom:** click and drag up or down on the magnifying glass icon below it.
- **Pan:** click and drag on the hand icon.
- **Frame the selection:** menu **View > Frame Selected**.

Two-finger scrolling on the touchpad may also orbit, and pinching may zoom
*(not tried here; depends on the touchpad driver)*. If you want Alt + drag to
orbit: **Edit > Preferences > Input > Emulate 3 Button Mouse**.

## 6. Make a test mesh

All of this happens with the mouse pointer over the 3D Viewport.

1. **Delete the cube:** click it (it gets an orange outline), press
   **Delete**.
2. **Add a monkey head:** menu **Add > Mesh > Monkey** in the viewport's
   header bar. A head called *Suzanne* appears and is selected.
3. **Make it denser:** press **Ctrl+2**. This adds a *Subdivision Surface*
   modifier at level 2 *(not tried here)*. You do not need to apply the
   modifier: the add-on remeshes the mesh as you see it, with modifiers (code:
   `_mesh_arrays` reads the evaluated mesh). It uses the modifier's
   *Levels Viewport* value. You can check the modifier in the Properties
   editor under the blue **wrench** tab.

Stay in **Object Mode** (the drop-down at the top left of the viewport says
"Object Mode"). The Remesh button is greyed out in any other mode, when the
selected object is not a mesh, or while a remesh is running.

## 7. Remesh

1. Press **N** with the pointer over the viewport (or menu **View >
   Sidebar**). A sidebar opens on the right of the viewport, with vertical
   tabs on its right edge. Click the **Quadami** tab.
2. Set **Target Quads** to 2000. Leave the other settings as they are.
3. Click **Remesh**. The status bar at the bottom shows
   `Quadami: 42% <step>   (Esc to cancel)` and the sidebar shows a progress
   bar. Blender stays usable meanwhile; try orbiting.
4. When it is done, a new object **Suzanne_remesh** appears and is selected,
   and Suzanne is hidden (in the Outliner its eye icon is closed; click the
   eye to show it again).
5. The sidebar shows the **report**: *Done*, *Quads: N of 2000 target*, then
   non-quads, non-manifold edges, winding flips, folded faces, empty islands,
   time, warnings. Rows in red are problems. Suzanne's eyes are separate
   pieces, so "Empty islands" should read 0 of 3 (head and two eyes; inferred,
   not run).

To look at the quads: press **Tab** to enter Edit Mode and see the wireframe,
**Tab** again to leave. Or in the viewport header, the **Overlays** drop-down,
tick **Wireframe**.

## 8. Undo

Press **Ctrl+Z** once. The remeshed object should disappear and Suzanne show
again *(undo of this add-on is not tried here)*.

## 9. Cancel

1. Click Suzanne (in the viewport or the Outliner) so it is selected.
2. Set **Target Quads** to 50000 and click **Remesh**.
3. After a few seconds press **Esc** (pointer over the Blender window), or
   click **Cancel** in the sidebar.
4. The status bar says *cancelling...*. Within 5 seconds the sidebar shows
   **Cancelled** and no new object is added. If the remesher does not stop by
   itself in 5 seconds, the add-on kills it.

## If something goes wrong

Copy the terminal's text (the part after `Quadami worker output:`) and the
status line in the sidebar, and send both. To quit Blender: **File > Quit**;
it asks whether to save, which you don't need for this test.
