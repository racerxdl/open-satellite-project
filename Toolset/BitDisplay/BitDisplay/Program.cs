using System;
using Gtk;

namespace BitDisplay
{
  class MainClass
  {
    public static void Main (string[] args)
    {
      Application.Init ();
      int width = -1;
      int offset = 0;
      bool bitMode = false;
      if (args.Length < 2) {
        Console.WriteLine ("Usage: ./BitDisplay.exe filename width [bitmode] [offset]");
        Environment.Exit(1);
      }

      string filename = args[0];
      int.TryParse (args [1], out width);
      if (args.Length >= 3) {
        int o;
        int.TryParse (args [2], out o);
        if (o == 1) {
          bitMode = true;
        }
        if (args.Length == 4) {
          int.TryParse (args [3], out offset);
        }
      }
      MainWindow win = new MainWindow (filename, width, offset, bitMode);
      win.Show ();
      Application.Run ();
    }
  }
}
