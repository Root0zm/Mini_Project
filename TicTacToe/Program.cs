
using System;

namespace TicTacToe
{
    class Program
    {
        private static char [, ] board = new char [3,3];
        static void Main (string [] args)   {
            TaoBang();
            HienThi();
        }
        static void TaoBang()
        {
            for(int i=0;i<3;i++)
            {
                for(int j=0;j<3;j++)
                   board[i,j]=' ';
            }
        }
        static void HienThi()
        {
            Console.WriteLine("  0 1 2");
            for(int i=0;i<3;i++)
            {
                Console.Write($"{i} ");
                for(int j=0;j<3;j++){
                    Console.Write(board[i,j]);
                    if(j<2) Console.Write ("|");
                }
                Console.WriteLine();
                if(i<2) Console.WriteLine("  -+-+-");
            }
        }
    }
}
