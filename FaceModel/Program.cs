using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FaceSdk;
using System.Drawing;
using PixelFormat = System.Drawing.Imaging.PixelFormat;

namespace FaceModel
{
    class Program
    {
        static void Main(string[] args)
        {
            FaceBeautyTests tester = new FaceBeautyTests();
            tester.TestBeautyScore(@"D:\Work\Code\scratch\FaceModel\Data\test-0.jpg");
            
            //DemoFacialFeature.DataSelection(@"D:\Work\FaceData\FaceData_Train\log\Train_F80s_F.tsv", @"D:\Work\FaceData\Face_BeautyGoPro\Log\Train_F80s_F_frontface.tsv");
            //DemoFacialFeature.DataSelection(@"D:\Work\FaceData\Face_BeautyGoPro\Log\Train_F80s_F_frontface.tsv", 
            //                                @"D:\Work\FaceData\Face_BeautyGoPro\Log\Train_F80s_F_frontface_train.tsv", 
            //                                @"D:\Work\FaceData\Face_BeautyGoPro\Log\Train_F80s_F_frontface_test.tsv",
            //                                200,
            //                                50);

            //var faces = new List<FaceInfo>();
            //using (var pf = new StreamReader(@"D:\Work\FaceData\Face_BeautyGoPro\Log\Train_F80s_F_frontface_test_68pt_std.txt"))
            //{

            //    string line;
            //    while ((line = pf.ReadLine()) != null)
            //    {
            //        var items = line.Split('\t');
            //        var face = new FaceInfo
            //        {
            //            OriImgPath = items[1],
            //            ThumbnailPath = items[2],
            //            BeautyLevelDict = new Dictionary<string, int> { { "F80s", Int32.Parse(items[9]) } },
            //            BeautyScoreDict = new Dictionary<string, float> { { "F80s", Single.Parse(items[10]) } },
            //            Age = Single.Parse(items[5])
            //        };

            //        face.SetFaceRectLandmarks(items[8], items[12].TrimEnd(' '));
            //        DemoFacialFeature.CalFacialFeature(face);

            //        faces.Add(face);
            //    }
            //}

            //using (var pf = new StreamWriter(@"D:\Work\FaceData\Face_BeautyGoPro\Log\Train_F80s_F_frontface_test_feature.txt"))
            //{
            //    foreach (var face in faces)
            //    {
            //        pf.Write(face.BeautyScoreDict["F80s"]);
            //        foreach (var feat in face.FacialFeature)
            //        {
            //            pf.Write("\t" + feat);
            //        }
            //        pf.Write("\n");
            //    }
            //}

        }

        
    }
}
