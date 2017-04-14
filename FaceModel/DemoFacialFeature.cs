using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FaceModel
{
    class DemoFacialFeature
    {
        public enum FeatureName
        {
            FaceWidth0 = 0,
            FaceWidth1,
            FaceWidth2,
            FaceHeight,
            FaceRatio0,
            FaceRatio1,
            FaceRatio2,
            FaceRatio3,
            NoseWidth,
            NoseHeight,
            NoseRatio,
            MouthWidth,
            MouthHeight,
            MouthRatio,
            InnerDistOfEyes,
            DistOfPupil,
            LeftEyeWidth,
            LeftEyeHeight,
            LeftEyeRatio,
            RightEyeWidth,
            RightEyeHeight,
            RightEyeRatio,
            LeftEyeBrowWidth,
            RightEyeBrowWidth,
            DistOfLeftEyeAndEyebrow,
            DistOfRightEyeAndEyebrow,
            DistOfEyeAndNose,
            DistOfNoseAndMouth,
            DistOfMouthAndChin,
            DistOfNoseAndChin,
            GoldenRatio0,
            GoldenRatio1,
            GoldenRatio2,
            GoldenRatio3,
            End,
        };

        static public void DataSelection(string sourcePath, string outPath)
        {
            var faceModel = FaceBasicFunc.Instance;
            faceModel.Reload();
            
            using (var pf = new StreamReader(sourcePath))
            using (var pfOut = new StreamWriter(outPath))
            {
                string line;
                while ((line = pf.ReadLine()) != null)
                {
                    var items = line.Split('\t');
                    
                    if(int.Parse(items[3]) != 1)
                        continue;

                    var imgPath = items[1];
                    var faceInfo = faceModel.FaceDetection(imgPath);

                    if(faceInfo.Count != 1)
                        continue;

                    if(Math.Abs(faceInfo[0].Pose.Roll) <= 5f && Math.Abs(faceInfo[0].Pose.Yaw) <= 5f)
                        pfOut.WriteLine(line);
                }
            }
        }

        public static void DataSelection(string sourcePath, string trainPath, string testPath, int trainNum, int testNum)
        {
            var trainCount = Enumerable.Repeat(trainNum, 5).ToList();
            var testCount = Enumerable.Repeat(testNum, 5).ToList();

            using (var pf = new StreamReader(sourcePath))
            using (var pfTrain = new StreamWriter(trainPath))
            using (var pfTest = new StreamWriter(testPath))
            {
                string line;
                while ((line = pf.ReadLine()) != null)
                {
                    var items = line.Split('\t');

                    var beautyLevel = int.Parse(items[9]);

                    if (trainCount[beautyLevel - 1] > 0)
                    {
                        pfTrain.WriteLine(line);
                        trainCount[beautyLevel - 1]--;
                    }
                    else if (testCount[beautyLevel - 1] > 0)
                    {
                        pfTest.WriteLine(line);
                        testCount[beautyLevel - 1]--;
                    }
                }
            }
        }

        public static void DrawLandmark(Bitmap img, FaceInfo faceinfo)
        {
            Graphics gImg = Graphics.FromImage(img);
            var pen = new Pen(Color.Red);
            foreach (var pt in faceinfo.Landmarks.Points)
            {
                gImg.DrawEllipse(pen, pt.X, pt.Y, 2, 2);
            }
        }

        public static void CalFacialFeature(FaceInfo faceinfo)
        {
            faceinfo.FacialFeature = new float[(int)FeatureName.End];
            var landmarks = faceinfo.Landmarks.Points;
            int idx;

            // face width
            idx = (int)FeatureName.FaceWidth0;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[0], landmarks[16]);

            idx = (int) FeatureName.FaceWidth1;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[3], landmarks[13]);

            idx = (int) FeatureName.FaceWidth2;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[5], landmarks[11]);

            // face height
            idx = (int)FeatureName.FaceHeight;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[0], landmarks[16], landmarks[8]);

            // face ratio
            idx = (int)FeatureName.FaceRatio0;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int)FeatureName.FaceWidth0] / faceinfo.FacialFeature[(int)FeatureName.FaceHeight];

            idx = (int) FeatureName.FaceRatio1;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int) FeatureName.FaceWidth0]/
                                          faceinfo.FacialFeature[(int) FeatureName.FaceWidth1];

            idx = (int)FeatureName.FaceRatio2;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int)FeatureName.FaceWidth0] /
                                          faceinfo.FacialFeature[(int)FeatureName.FaceWidth2];

            idx = (int)FeatureName.FaceRatio3;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int)FeatureName.FaceWidth1] /
                                          faceinfo.FacialFeature[(int)FeatureName.FaceWidth2];

            // nose width
            idx = (int)FeatureName.NoseWidth;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[31], landmarks[35]);

            // nose height
            idx = (int)FeatureName.NoseHeight;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[27], landmarks[33]);

            // nose ratio
            idx = (int)FeatureName.NoseRatio;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int)FeatureName.NoseWidth] / faceinfo.FacialFeature[(int)FeatureName.NoseHeight];

            // mouth width
            idx = (int)FeatureName.MouthWidth;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[48], landmarks[54]);

            // mouth height
            idx = (int)FeatureName.MouthHeight;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[51], landmarks[57]);

            // mouth ratio
            idx = (int)FeatureName.MouthRatio;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int)FeatureName.MouthWidth] / faceinfo.FacialFeature[(int)FeatureName.MouthHeight];

            // distance between inner edge of eyes
            idx = (int)FeatureName.InnerDistOfEyes;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[39], landmarks[42]);

            // distance between two pupils
            idx = (int) FeatureName.DistOfPupil;
            faceinfo.FacialFeature[idx] = CalDist(CalMidPoint(landmarks[36], landmarks[39]), CalMidPoint(landmarks[42], landmarks[45]));

            // left eye width
            idx = (int)FeatureName.LeftEyeWidth;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[36], landmarks[39]);

            // left eye height
            idx = (int)FeatureName.LeftEyeHeight;
            faceinfo.FacialFeature[idx] = CalDist(CalMidPoint(landmarks[37], landmarks[38]), CalMidPoint(landmarks[40], landmarks[41]));

            // left eye ratio
            idx = (int)FeatureName.LeftEyeRatio;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int)FeatureName.LeftEyeWidth] / faceinfo.FacialFeature[(int)FeatureName.LeftEyeHeight];

            // right eye width
            idx = (int)FeatureName.RightEyeWidth;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[42], landmarks[45]);

            // right eye height
            idx = (int)FeatureName.RightEyeHeight;
            faceinfo.FacialFeature[idx] = CalDist(CalMidPoint(landmarks[43], landmarks[44]), CalMidPoint(landmarks[46], landmarks[47]));

            // right eye ratio
            idx = (int)FeatureName.RightEyeRatio;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int)FeatureName.RightEyeWidth] / faceinfo.FacialFeature[(int)FeatureName.RightEyeHeight];

            // left eyebrow width
            idx = (int)FeatureName.LeftEyeBrowWidth;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[17], landmarks[21]);

            // right eyebrow width
            idx = (int)FeatureName.RightEyeBrowWidth;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[22], landmarks[26]);

            // distance between left eye and eyebrow
            idx = (int)FeatureName.DistOfLeftEyeAndEyebrow;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[36], landmarks[39], landmarks[19]);

            // distance between right eye and eyebrow
            idx = (int)FeatureName.DistOfRightEyeAndEyebrow;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[42], landmarks[45], landmarks[24]);

            // distance between nose and eye
            idx = (int)FeatureName.DistOfEyeAndNose;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[39], landmarks[42], landmarks[30]);

            // distance between nose and mouth
            idx = (int)FeatureName.DistOfNoseAndMouth;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[48], landmarks[54], landmarks[30]);

            // distance between mouth and chin
            idx = (int)FeatureName.DistOfMouthAndChin;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[48], landmarks[54], landmarks[8]);

            // distance between nose and chin
            idx = (int) FeatureName.DistOfNoseAndChin;
            faceinfo.FacialFeature[idx] = CalDist(landmarks[30], landmarks[8]);

            // golden ratio: D(eye, nose) / D(nose, chin)
            idx = (int)FeatureName.GoldenRatio0;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int) FeatureName.DistOfEyeAndNose]/
                                          faceinfo.FacialFeature[(int) FeatureName.DistOfNoseAndChin];

            // golden ratio: D(nose, mouth) / D(mouth, chin)
            idx = (int) FeatureName.GoldenRatio1;
            faceinfo.FacialFeature[idx] = faceinfo.FacialFeature[(int) FeatureName.DistOfNoseAndMouth]/
                                          faceinfo.FacialFeature[(int) FeatureName.DistOfMouthAndChin];

            // golden ratio: |D(inner eye) - nose width|
            idx = (int) FeatureName.GoldenRatio2;
            faceinfo.FacialFeature[idx] = Math.Abs(faceinfo.FacialFeature[(int) FeatureName.InnerDistOfEyes] -
                                                   faceinfo.FacialFeature[(int) FeatureName.NoseWidth]);

            // golden ratio: |left eye width - right eye width|
            idx = (int)FeatureName.GoldenRatio3;
            faceinfo.FacialFeature[idx] = Math.Abs(faceinfo.FacialFeature[(int) FeatureName.LeftEyeWidth] -
                                                   faceinfo.FacialFeature[(int) FeatureName.RightEyeWidth]);

        }

        // distance of two points
        private static float CalDist(FaceSdk.PointF pt0, FaceSdk.PointF pt1)
        {
            return (float)Math.Sqrt(Math.Pow(pt1.X - pt0.X, 2) + Math.Pow(pt1.Y - pt0.Y, 2));
        }

        // distance of point to line D[line(pt0, pt1), pt2]
        private static float CalDist(FaceSdk.PointF pt0, FaceSdk.PointF pt1, FaceSdk.PointF pt2)
        {
            var a = pt1.Y - pt0.Y;
            var b = pt1.X - pt0.X;
            var c1 = pt1.X*pt0.Y - pt1.Y*pt0.X;
            var c2 = (pt1.X - pt0.X)*pt2.Y - (pt1.Y - pt0.Y)*pt2.X;

            var dist = Math.Abs(c1 - c2)/Math.Sqrt(Math.Pow(a, 2) + Math.Pow(b, 2));

            return (float) dist;
        }

        private static FaceSdk.PointF CalMidPoint(FaceSdk.PointF pt0, FaceSdk.PointF pt1)
        {
            return new FaceSdk.PointF()
            {
                X = (pt0.X + pt1.X) / 2,
                Y = (pt0.Y + pt1.Y) / 2
            };
        }

        public static float[] CalCorrelationCoefficient(List<FaceInfo> faces)
        {
            var corrcoef = new float[(int)FeatureName.End];
            var beautyScore = new List<float>();
            var facialFeature = new List<List<float>>();
            for (int i = 0; i < (int) FeatureName.End; i++)
            {
                facialFeature.Add(new List<float>());
            }
            
            var n = faces.Count;
            foreach (var face in faces)
            {
                beautyScore.Add(ScoreMapping(face.BeautyScoreDict["F80s"]));
                for (int i = 0; i < (int)FeatureName.End; i++)
                {
                    facialFeature[i].Add(face.FacialFeature[i]);
                }
            }

            float meanBeautyScore = beautyScore.Average();
            float stdBeautyScore = (float)Math.Sqrt(CalVariance(beautyScore, meanBeautyScore));

            // calculate standard variance
            for (int i = 0; i < (int) FeatureName.End; i++)
            {
                float meanFacialFeature = facialFeature[i].Average();
                float stdFacialFeature = (float)Math.Sqrt(CalVariance(facialFeature[i], meanFacialFeature));
                float cov = CalCovariance(beautyScore, facialFeature[i], meanBeautyScore, meanFacialFeature);

                corrcoef[i] = (cov / (stdBeautyScore * stdFacialFeature));

                Console.WriteLine("{0:0.000}", corrcoef[i]);
            }

            return corrcoef;
        }

        private static float CalVariance(List<float> x, float mu)
        {
            return x.Sum(xi => (xi - mu) * (xi - mu));
        }

        private static float CalCovariance(List<float> x, List<float> y, float muX, float muY)
        {
            if (x.Count != y.Count || x.Count <= 0 || y.Count <= 0)
                return 0;

            var n = x.Count;
            float cov = 0f;
            for (var i = 0; i < n; i++)
            {
                cov += (x[i] - muX) * (y[i] - muY);
            }

            return cov;
        }

        private static float ScoreMapping(float beautyscore)
        {
            // map the original beauty score (1,5) to (1-10)
            if (beautyscore < 1)
                beautyscore = 1;
            else if (beautyscore > 5)
                beautyscore = 5;

            if (beautyscore <= 3)
                beautyscore = 11 - (float)Math.Exp(0.695 * (beautyscore - 1));
            else if (beautyscore < 4.25)
                beautyscore = 8f - (beautyscore - 2.5f) * 2f;
            else
                beautyscore = 5f - (beautyscore - 4f) * 4f;

            beautyscore = (float)Math.Round(beautyscore, 1);

            if (beautyscore < 1)
                beautyscore = 1f;

            return beautyscore;
        }
    }
}
