using FaceSdk;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.ProjectOxford.Emotion.Contract;

namespace FaceModel
{
    class FaceBeautyTests
    {
        private readonly FaceBasicFunc _faceBasicFunc = FaceBasicFunc.Instance;
        private readonly FaceBeautyModel _faceBeautyModel = FaceBeautyModel.Instance;

        public FaceBeautyTests()
        {
            _faceBasicFunc.Reload();
            _faceBeautyModel.Reload();
        }

        public List<FaceInfo> TestBeautyScore(string pathQuery)
        {
            var watch = Stopwatch.StartNew();
            List<FaceInfo> testInfo = _faceBasicFunc.FaceDetection(pathQuery);
            Console.WriteLine("[Face Detection]: {0:0.00}ms", watch.ElapsedMilliseconds);

            FaceSdk.IImage colorImage = ImageUtility.LoadImageFromBitmapAsRgb24(new Bitmap(pathQuery));
            watch = Stopwatch.StartNew();
            foreach (var face in testInfo)
            {
                face.FaceFeat = _faceBasicFunc.FeatureExtraction(colorImage, face.Landmarks, new string[] {"beauty"});
                face.BeautyScoreDict = _faceBeautyModel.PredictCNN(face.FaceFeat);
            }
            Console.WriteLine("[Beauty Prediction]: {0:0.00}ms", watch.ElapsedMilliseconds);

            return testInfo;
        }

        public List<FaceInfo> BatchTestBeautyScoreCNN(string pathQueryList, string pathResult = null)
        {
            float avgTimeDetection = 0f;
            var faceInfo = new List<FaceInfo>();

            int n = 0;
            StreamReader pf = new StreamReader(pathQueryList);
            string line;
            while ((line = pf.ReadLine()) != null)
            {
                if (n % 20 == 0)
                    Console.Write("\n [Face Detection] ");
                Console.Write("{0} ", n++);

                // face detection                
                try
                {
                    var watch = Stopwatch.StartNew();
                    var faces = _faceBasicFunc.FaceDetection(line);
                    watch.Stop();
                    avgTimeDetection += watch.ElapsedMilliseconds;

                    int i = 0;
                    FaceSdk.IImage colorImage = ImageUtility.LoadImageFromBitmapAsRgb24(new Bitmap(line));
                    foreach(var face in faces)
                    {
                        face.Key = string.Format("{0}_{1}", Path.GetFileNameWithoutExtension(line), i++);
                        face.OriImgPath = line;
                        face.FaceFeat = _faceBasicFunc.FeatureExtraction(colorImage, face.Landmarks, new string[] { "beauty" });
                    }

                    faceInfo.AddRange(faces);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.ToString());
                }

                //if (faceInfo.Count == 5)
                //    break;
            }
            pf.Close();
            Console.WriteLine("\nAvg Running Time of Detection: {0:0.000}ms", avgTimeDetection /= n);

            _faceBeautyModel.PredictCNN(ref faceInfo);

            // write to file
            if (pathResult != null)
                WriteResultToFile(faceInfo, pathResult);

            return faceInfo;
        }

        static private void WriteResultToFile(List<FaceInfo> faceInfo, string pathResult)
        {
            // write results to file
            using (StreamWriter pf = new StreamWriter(pathResult))
            {
                foreach (var face in faceInfo)
                {
                    pf.Write(face.Key + '\t' + face.OriImgPath + '\t');

                    foreach (var labeler in face.BeautyScoreDict.Keys)
                        pf.Write(string.Format("{0}:{1:0.000} ", labeler, face.BeautyScoreDict[labeler]));

                    pf.Write("\n");
                }
            }
        }

        static private bool Measure(List<FaceInfo> testInfo, List<Dictionary<string, float>> gtBeautyScore, out Dictionary<string, float> loss, out Dictionary<string, int> num_false)
        {
            if (testInfo.Count() != gtBeautyScore.Count())
                throw (new Exception("[ERROR] in Measure(List<FaceBeautyInfo> testInfo, List<float> gtBeautyScore, out float loss, out int num_false)"));

            loss = new Dictionary<string, float>();
            num_false = new Dictionary<string,int>();

            foreach (var testLabeler in testInfo[0].BeautyScoreDict.Keys)
            {
                foreach (var gtLabeler in gtBeautyScore[0].Keys)
                {
                    int num = 0;
                    float l = 0f;

                    for (int i = 0; i < testInfo.Count(); i++)
                    {
                        // measure: least square
                        if (Math.Abs(testInfo[i].BeautyScoreDict[testLabeler] - gtBeautyScore[i][gtLabeler]) > 2f)
                            num++;
                        l += (testInfo[i].BeautyScoreDict[testLabeler] - gtBeautyScore[i][gtLabeler]) * (testInfo[i].BeautyScoreDict[testLabeler] - gtBeautyScore[i][gtLabeler]);
                    }

                    l = Convert.ToSingle(Math.Sqrt(l / testInfo.Count()));

                    num_false.Add(string.Format("{0}_{1}", testLabeler, gtLabeler), num);
                    loss.Add(string.Format("{0}_{1}", testLabeler, gtLabeler), l);
                }
            }            

            return true;
        }
        
        public void ScoreMappingAnalysis(string resultPath)
        {
            Dictionary<string, float[]> hist = new Dictionary<string, float[]>();
            var lines = File.ReadAllLines(resultPath).ToList();
            int n = lines.Count;

            foreach (var line in lines)
            {
                var items = line.Split('\t')[2].TrimEnd().Split(' ');
                foreach (var item in items)
                {
                    var view = item.Split(':')[0];
                    var score = Convert.ToSingle(item.Split(':')[1]);

                    if (!hist.ContainsKey(view))
                    {
                        var value = new float[6];
                        value[ScoreToLevel(score)] += 1f/n;
                        hist.Add(view, value);
                    }
                    else
                    {
                        var value = hist[view];
                        value[ScoreToLevel(score)] += 1f / n;
                        hist[view] = value;
                    }
                }
            }
            
            foreach (var view in hist.Keys)
            {
                Console.Write("\n" + view + ":");
                foreach (var perc in hist[view])
                {
                    Console.Write("{0:0.00%} ", perc);
                }
                Console.Write('\n');
            }
        }

        static private int ScoreToLevel(float score)
        {
            return Math.Min(5, (int)Math.Floor((Convert.ToDecimal(score) - 1) / 0.8m) + 1);
        }
        
    }
}
