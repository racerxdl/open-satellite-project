using System;
using Gtk;
using System.Drawing;
using Gdk;
using System.IO;

public partial class MainWindow: Gtk.Window
{
  public MainWindow (string filename, int width, int offset, bool bitMode) : base (Gtk.WindowType.Toplevel)
  {
    Build ();
    int height = 0;
    long length = new System.IO.FileInfo (filename).Length;
    height = (int)Math.Ceiling ((double)length / width);
    if (height > 800) {
      height = 800;
      length = 800 * width;
    }
    Pixbuf buff;

    if (bitMode) {
      width *= 8;
      byte[] data = new byte[width * height * 3 ];
      //height /= 8;

      using (BinaryReader reader = new BinaryReader (File.Open (filename, FileMode.Open))) {
        reader.ReadBytes (offset);
        for (int i = 0; i < length - offset; i++) {
          try {
            byte b = reader.ReadByte ();
            for (int p = 0; p < 8; p++) {
              byte bit = ((b >> p) & 1) == 1 ? (byte)255 : (byte)0;
              data [i * 3 * 8 + 0 + p * 3] = bit;
              data [i * 3 * 8 + 1 + p * 3] = bit;
              data [i * 3 * 8 + 2 + p * 3] = bit;
            }
          } catch (Exception) {
            break;
          }
        }
      }
      buff = new Pixbuf (data, false, 8, width, height, width * 3);
    } else {
      byte[] data = new byte[width * height];

      using (BinaryReader reader = new BinaryReader (File.Open (filename, FileMode.Open))) {
        reader.ReadBytes (offset);
        for (int i = 0; i < length - offset; i++) {
          byte b = reader.ReadByte ();
          data [i] = b;
        }
      }
      buff = new Pixbuf (data, false, 8, width, height, width);
    }
    this.GdkWindow.Resize (width > 1920 ? 1920 : width, height);

    canvas.ExposeEvent += (o, args) => {
      Gdk.GC gc = new Gdk.GC ((Drawable)canvas.GdkWindow);
      canvas.GdkWindow.DrawPixbuf (gc, buff, 0, 0, 0, 0, width, height, RgbDither.Max, 0, 0);
    };
  }

  protected void OnDeleteEvent (object sender, DeleteEventArgs a)
  {
    Application.Quit ();
    a.RetVal = true;
  }
}
