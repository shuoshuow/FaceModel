using FaceSdk;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;

namespace FaceModel
{
    class FaceBeautyModel
    {
        private static readonly FaceBeautyModel instance = new FaceBeautyModel();
        public static FaceBeautyModel Instance { get { return instance; } }

        private Dictionary<string, FaceAttributeBeautyIndex> _faceBeautyCnn = null;
        private readonly string[] _allLabelers = { "F80s", "F90s", "M80s", "M90s", "F80sUS", "Jap", "FKR", "MInd", "MUK" };

        public void Reload()
        {
            try
            {
                // load beauty CNN model
                _faceBeautyCnn = new Dictionary<string, FaceAttributeBeautyIndex>();
                foreach (var labeler in _allLabelers)
                {
                    string modelPath = Path.Combine(@".\Models", string.Format("Model_beauty_small_{0}_facesdk.bin", labeler));
                    if (File.Exists(modelPath))
                    {
                        _faceBeautyCnn.Add(labeler, new FaceAttributeBeautyIndex(new Model(modelPath)));
                        continue;
                    }
                    modelPath = Path.Combine(@".\Models", string.Format("Model_beauty_small_{0}_facesdk.mdl", labeler));
                    if (File.Exists(modelPath))
                        _faceBeautyCnn.Add(labeler, new FaceAttributeBeautyIndex(new Model(modelPath)));
                }
            }
            catch (Exception e)
            {
                Trace.TraceError("Error loading FaceBeauty model: {0}", e.ToString());
                this._faceBeautyCnn = null;
            }
        }

        public Dictionary<string, float> PredictCNN(FaceFeatureCNN faceFeature)
        {
            var beautyScoreDict = new Dictionary<string, float>();

            if (this._faceBeautyCnn == null)
                Reload();
            
            foreach (var labeler in _allLabelers)
            {
                if (_faceBeautyCnn.ContainsKey(labeler))
                {
                    float s = _faceBeautyCnn[labeler].Analyze(faceFeature);
                    beautyScoreDict.Add(labeler, s);
                }
            }
            //watch.Stop();
            //Console.WriteLine("Runtime of Beauty Score: {0:0.000}ms", watch.ElapsedMilliseconds);

            return beautyScoreDict;
        }

        public void PredictCNN(ref List<FaceInfo> faceInfo)
        {
           // float avgTimeBeauty = 0f;
            if (this._faceBeautyCnn == null)
                Reload();
            
            int n = 0;
            foreach (var face in faceInfo)
            {
                if (n % 20 == 0)
                    Console.Write("\n [Beauty Score Prediction] ");
                Console.Write("{0} ", n++);

                if(face.FaceFeat == null)
                    continue;

                //var watch = Stopwatch.StartNew();
                face.BeautyScoreDict = PredictCNN(face.FaceFeat);
                //watch.Stop();
                //avgTimeBeauty += watch.ElapsedMilliseconds;
            }
            //Console.WriteLine("\nAvg Runtime of Beauty Score: {0:0.000}ms", avgTimeBeauty / n);
        }
    }
}
