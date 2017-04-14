using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.ProjectOxford.Emotion.Contract;

namespace FaceModel
{
    class EmotionApiTests
    {
        private readonly FaceBasicFunc _faceBasicFunc = FaceBasicFunc.Instance;
        private readonly FaceEmotionModel _faceEmotion = FaceEmotionModel.Instance;

        public EmotionApiTests()
        {
            _faceBasicFunc.Reload();
            _faceEmotion.Reload();
        }

        public void TestEmotionApi_01()
        {
            // read image list
            var imgFolder = @"D:\Work\FaceData\Face_Emotion\Test_web\Images";
            var imgList = new List<string>();
            imgList.AddRange(Directory.GetFiles(imgFolder, "*.jpg").ToList<string>());
            imgList.AddRange(Directory.GetFiles(imgFolder, "*.jpeg").ToList<string>());
            imgList.AddRange(Directory.GetFiles(imgFolder, "*.bmp").ToList<string>());
            imgList.AddRange(Directory.GetFiles(imgFolder, "*.png").ToList<string>());
            imgList.AddRange(Directory.GetFiles(imgFolder, "*.gif").ToList<string>());

            // predict face emotion and write result to file
            using (var pf = new StreamWriter(@"D:\Work\FaceData\Face_Emotion\Test_web\test_result.tsv"))
            {
                foreach (var imgPath in imgList)
                {
                    List<FaceInfo> faceInfo = TestEmotionApi(imgPath);

                    if(faceInfo.Count == 0)
                        pf.Write(string.Format("{0}\tNo Face\n", imgPath));
                    foreach (var face in faceInfo)
                    {
                        var left = face.BoundingBox.Left;
                        var top = face.BoundingBox.Top;
                        var right = face.BoundingBox.Left + face.BoundingBox.Width;
                        var bottom = face.BoundingBox.Top + face.BoundingBox.Height;

                        pf.Write(string.Format("{0}\t{1} {2} {3} {4}", imgPath, left, top, right, bottom));
                        if(!string.IsNullOrEmpty(face.DomiEmotion))
                            pf.Write(string.Format("\t{0}", face.DomiEmotion));
                        pf.Write("\n");
                    }
                }
            }
        }

        public List<FaceInfo> TestEmotionApi(string imgPath)
        {
            //imgPath = @"D:\Work\FaceData\Face_Emotion\CEEBBA7E8219D8681D70F9C07D10AB65.jpg";
            List<FaceInfo> faceInfo = _faceBasicFunc.FaceDetection(imgPath);
            Stream stream = new FileStream(imgPath, FileMode.Open, FileAccess.Read);

            foreach (var face in faceInfo)
            {
                var faceRect = new Microsoft.ProjectOxford.Common.Rectangle
                {
                    Left = face.BoundingBox.Left,
                    Top = face.BoundingBox.Top,
                    Height = face.BoundingBox.Height,
                    Width = face.BoundingBox.Width
                };

                face.EmotionScore = _faceEmotion.PredictCNN(stream, faceRect);

                if (face.EmotionScore != null)
                {
                    Console.WriteLine(string.Format("Rectanle: ({0}, {1}, {2}, {3})", faceRect.Left, faceRect.Top,
                        faceRect.Width, faceRect.Height));
                    var score = face.EmotionScore.Scores;
                    Console.Write("Emotion: ");
                    Console.Write(string.Format("Neutral:{0:0.000}", score.Neutral));
                    Console.Write(string.Format(" | Happy:{0:0.000}", score.Happiness));
                    Console.Write(string.Format(" | Superise:{0:0.000}", score.Surprise));
                    Console.Write(string.Format(" | Sadness:{0:0.000}", score.Sadness));
                    Console.Write(string.Format(" | Anger:{0:0.000}", score.Anger));
                    Console.Write(string.Format(" | Contempt:{0:0.000}", score.Contempt));
                    Console.Write(string.Format(" | Disgust:{0:0.000}", score.Disgust));
                    Console.Write(string.Format(" | Fear:{0:0.000}", score.Fear));
                    Console.Write("\n");

                    // find dominate emotion
                    string domiEmotion = string.Empty;
                    float highScore = -1f;
                    FindDominateEmotion(score, ref highScore, ref domiEmotion);
                    Console.WriteLine(string.Format("Dominate emotion: {0} ({1:0.000})", domiEmotion, highScore));

                    face.DomiEmotionScore = highScore;
                    face.DomiEmotion = domiEmotion;
                }
            }

            return faceInfo;
        }

        public void BatchTestEmotionAPi()
        {
            var emotionLabel = new Dictionary<string, int>{ 
                                                            {"Neutral", 0}, 
                                                            {"Happiness", 1},
                                                            {"Surprise", 2},
                                                            {"Sadness", 3},
                                                            {"Anger", 4},
                                                            {"Contempt", 5},
                                                            {"Disgust", 6},
                                                            {"Fear", 7},
                                                            {"Other", 8},
                                                        };

            FaceEmotionModel _faceEmotion = FaceEmotionModel.Instance;
            _faceEmotion.Reload();

            float[,] confMatrix = new float[8, 8];
            int[] totalNum = new int[8];
            int N = 0;
            float AP = 0f;
            float avgTimePredict = 0f;

            using (var pf = new StreamReader(@"D:\Work\FaceData\Face_Emotion\Test_web\test_gt.csv"))
            using (var pfError = new StreamWriter(@"D:\Work\FaceData\Face_Emotion\Test_web\test_error.txt"))
            using (var pfWrong = new StreamWriter(@"D:\Work\FaceData\Face_Emotion\Test_web\test_wrong.txt"))
            {
                var line = String.Empty;
                while ((line = pf.ReadLine()) != null)
                {
                    var items = line.Split(',');
                    string imgPath = items[0];
                    string faceRect = items[1];
                    string gtEmotion = items[2];

                    var img = new Bitmap(imgPath);

                    var faceRects = new Microsoft.ProjectOxford.Common.Rectangle[1];
                    faceRects[0] = new Microsoft.ProjectOxford.Common.Rectangle()
                    {
                        Left = Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[0])),
                        Top = Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[1])),
                        Width = Math.Min(img.Width, Convert.ToInt32(faceRect.Split(' ')[2])) - Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[0])),
                        Height = Math.Min(img.Height, Convert.ToInt32(faceRect.Split(' ')[3])) - Math.Max(0, Convert.ToInt32(faceRect.Split(' ')[1]))
                    };

                    if (faceRects[0].Width < 36 || faceRects[0].Height < 36)
                    {
                        pfError.WriteLine(imgPath + "\t small face");
                        continue;
                    }

                    try
                    {
                        var watch = Stopwatch.StartNew();
                        Emotion[] emotions = _faceEmotion.PredictCNN(imgPath, faceRects);
                        watch.Stop();
                        avgTimePredict += watch.ElapsedMilliseconds;

                        float highScore = -1f;
                        string domiEmotion = string.Empty;
                        FindDominateEmotion(emotions[0].Scores, ref highScore, ref domiEmotion);

                        int i = emotionLabel[domiEmotion];
                        int j = emotionLabel[gtEmotion];
                        confMatrix[i, j]++;
                        totalNum[j]++;
                        N++;

                        if (domiEmotion != gtEmotion)
                            pfWrong.WriteLine(string.Format("{0}\t{1}\t{2}", Path.GetFileName(imgPath), domiEmotion, gtEmotion));

                        if (N % 500 == 0)
                            Console.WriteLine(string.Format("\n{0} has been processed", N));
                        else if (N % 50 == 0)
                            Console.Write(".");

                        //if (N == 100)
                        //    break;
                    }
                    catch (Exception)
                    {
                        pfError.WriteLine(imgPath + "\t exception");
                    }
                }

                for (int i = 0; i < 8; i++)
                    for (int j = 0; j < 8; j++)
                    {
                        if (totalNum[j] != 0)
                            confMatrix[i, j] /= totalNum[j];
                        else
                            confMatrix[i, j] = 0;
                    }

                for (int i = 0; i < 8; i++)
                    AP += confMatrix[i, i] / 8;

                Console.Write('\n');
                for (int i = 0; i < 8; i++)
                {
                    for (int j = 0; j < 8; j++)
                        Console.Write(string.Format("{0:0.00}  ", confMatrix[i, j]));
                    Console.Write('\n');
                }
                Console.Write("total num");
                for (int j = 0; j < 8; j++)
                    Console.Write(string.Format("{0}  ", totalNum[j]));
                Console.Write('\n');
                Console.WriteLine(string.Format("AP = {0:0.00%}, N = {1}", AP, N));
                Console.WriteLine("\nAvg Running Time of Emotion Prediction: {0:0.000}ms", avgTimePredict /= N);
            }
        }


        private static void FindDominateEmotion(Scores score, ref float highScore, ref string domiEmotion)
        {
            highScore = score.Neutral;
            domiEmotion = "Neutral";

            if (score.Happiness > highScore)
            {
                highScore = score.Happiness;
                domiEmotion = "Happiness";
            }

            if (score.Surprise > highScore)
            {
                highScore = score.Surprise;
                domiEmotion = "Surprise";
            }

            if (score.Sadness > highScore)
            {
                highScore = score.Sadness;
                domiEmotion = "Sadness";
            }

            if (score.Anger > highScore)
            {
                highScore = score.Anger;
                domiEmotion = "Anger";
            }

            if (score.Contempt > highScore)
            {
                highScore = score.Contempt;
                domiEmotion = "Contempt";
            }

            if (score.Disgust > highScore)
            {
                highScore = score.Disgust;
                domiEmotion = "Disgust";
            }

            if (score.Fear > highScore)
            {
                highScore = score.Fear;
                domiEmotion = "Fear";
            }
        }
    }
}
