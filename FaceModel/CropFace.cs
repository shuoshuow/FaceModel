using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FaceModel
{
    class CropFace
    {
        public static void CropFaceRegionFromImage()
        {
            string targetPath = @"D:\Work\FaceData\Face_Emotion\Test_web\FaceImages";
            using (var pf = new StreamReader(@"D:\Work\FaceData\Face_Emotion\Test_web\test_gt.csv"))
            {
                string line;
                while ((line = pf.ReadLine()) != null)
                {
                    var items = line.Split(',');
                    var imgPath = items[0];
                    var faceRect = items[1];

                    var img = new Bitmap(imgPath);
                    System.Drawing.Rectangle cropRect = new System.Drawing.Rectangle()
                    {
                        X = Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[0])),
                        Y = Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[1])),
                        Width = Math.Min(img.Width, Convert.ToInt32(faceRect.Split(' ')[2])) - Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[0])),
                        Height = Math.Min(img.Height, Convert.ToInt32(faceRect.Split(' ')[3])) - Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[1]))
                    };
                    Bitmap faceImg = new Bitmap(cropRect.Width, cropRect.Height);
                    using (var gimg = Graphics.FromImage(faceImg))
                    {
                        gimg.DrawImage(img, new System.Drawing.Rectangle(0,0,cropRect.Width, cropRect.Height), cropRect, GraphicsUnit.Pixel);
                    }

                    if (!Directory.Exists(Path.Combine(targetPath, items[2])))
                        Directory.CreateDirectory(Path.Combine(targetPath, items[2]));
                    faceImg.Save(Path.Combine(targetPath, items[2], string.Format("{0}_{1}.jpg",Path.GetFileNameWithoutExtension(imgPath), faceRect)));
                    
                }
            }
        }
    }
}
