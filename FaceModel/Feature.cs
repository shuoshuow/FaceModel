using FaceSdk;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;

namespace FaceModel
{
    
    class Feature
    {
        private static readonly Feature instance = new Feature();
        public static Feature Instance { get { return instance; } }
        
        private FaceRecognitionCNN _recognizerCnn = null;        
        private FaceRecognitionCNN _recognizerCnnMouth = null;
        private FaceRecognitionCNN _recognizerCnnNose = null;
        private FaceRecognitionCNN _recognizerCnnEye = null;

        public bool Initialized
        {
            get
            {
                return (this._recognizerCnn != null && this._recognizerCnnMouth != null && this._recognizerCnnNose != null && this._recognizerCnnEye != null);
            }
        }

        public void Reload()
        {
            try
            {               
                _recognizerCnn = new FaceRecognitionCNN(new Model(@".\Models\ProductRecognitionCnn27pts.mdl"));
                //recognizerCNNMouth = new FaceRecognitionCNN(new Model(@".\Models\Model_facesim_mouth_facesdk.bin"));
                //recognizerCNNNose = new FaceRecognitionCNN(new Model(@".\Models\Model_facesim_nose_facesdk.bin"));
                //recognizerCNNEye = new FaceRecognitionCNN(new Model(@".\Models\Model_facesim_eyes_facesdk.bin"));
            }
            catch (Exception e)
            {
                Trace.TraceError("Error loading FaceBeauty model: {0}", e.ToString());
                this._recognizerCnn = null;
                this._recognizerCnnMouth = null;
                this._recognizerCnnNose = null;
                this._recognizerCnnEye = null;
            }
        }

        public FaceFeature ExtractFaceSDKFeat(FaceSdk.IImage colorImage, FaceSdk.FaceLandmarks landmarks, string component = "Face")
        {
            FaceFeature feat;

            switch (component)
            {
                case "Face":
                    feat = _recognizerCnn.ExtractFaceFeature(colorImage, landmarks, new List<FaceRecognitionCNN.Usage>() { FaceRecognitionCNN.Usage.BeautyIndex, FaceRecognitionCNN.Usage.Gender, FaceRecognitionCNN.Usage.Age });
                    break;
                case "Mouth":
                    feat = _recognizerCnnMouth.ExtractFaceFeature(colorImage, landmarks);
                    break;
                case "Nose":
                    feat = _recognizerCnnNose.ExtractFaceFeature(colorImage, landmarks);
                    break;
                case "Eye":
                    feat = _recognizerCnnEye.ExtractFaceFeature(colorImage, landmarks);
                    break;
                default:
                    throw (new Exception(String.Format("The input component name ({0}) is invalid!!!", component)));
            }

            return feat;
        }

        public void ExtractFaceSDKFeat(string pathImageList, string pathOutput, string component = "Face")
        {
            Console.WriteLine("\n\nExtracting Features...");

            int n = 0;

            using(var pfImageList = new StreamReader(pathImageList))
            using(var writer = new BinaryWriter(File.Open(pathOutput, FileMode.Create)))
            {
                string line;
                while ((line = pfImageList.ReadLine()) != null)
                {
                    Console.Write("{0} ", n);
                    if (n != 0 && n % 10 == 0)
                        Console.Write('\n');
                    n++;

                    // imagelist schema:
                    // [image name]\t[original image path]\t[thumbnail image path]\t[total face number]\t[face index]\t[age]\t[gender]\t[landmark]\t[bounding box]\t[beauty level]
                    var item = line.Split('\t');

                    // set face basic info
                    FaceInfo faceInfo = new FaceInfo();
                    faceInfo.SetFaceRectLandmarks(item[8].Trim(), item[7].Trim());

                    // extrac feature
                    string pathImage = item[1];
                    FaceSdk.IImage colorImage = ImageUtility.LoadImageFromBitmapAsRgb24(new Bitmap(pathImage));
                    FaceFeature feat = ExtractFaceSDKFeat(colorImage, faceInfo.Landmarks, component);

                    // write to file                
                    writer.Write(String.Format("{0}_{1}", item[0], item[4]));
                    writer.Write(feat.Vector);
                }
            }
        }

        public List<FaceInfo> LoadFaceSDKFeat(string pathFeat)
        {
            var faceInfo = new List<FaceInfo>();

            using (var reader = new BinaryReader(File.Open(pathFeat, FileMode.Open)))
            {
                while (reader.BaseStream.Position != reader.BaseStream.Length)
                {
                    var info = new FaceInfo();
                    FaceFeatureCNN feat = new FaceFeatureCNN(null);
                    info.Key = reader.ReadString();
                    feat.Vector = reader.ReadBytes(512);
                    info.FaceFeat = feat;

                    faceInfo.Add(info);
                }
            }

            return faceInfo;
        }

        public List<FaceInfo> LoadFaceSDKFeat(string pathFeat, string pathImageList, string labeler)
        {
            var faceInfo = new List<FaceInfo>();
            if (!File.Exists(pathFeat) || !File.Exists(pathImageList))
                return faceInfo;
            
            Dictionary<string, FaceInfo> dict = new Dictionary<string, FaceInfo>();
            using (BinaryReader reader = new BinaryReader(File.Open(pathFeat, FileMode.Open)))
            {
                while (reader.BaseStream.Position != reader.BaseStream.Length)
                {
                    FaceInfo info = new FaceInfo();
                    FaceFeatureCNN feat = new FaceFeatureCNN(null);

                    info.Key = reader.ReadString();
                    feat.Vector = reader.ReadBytes(512);
                    info.FaceFeat = feat;

                    if (!dict.ContainsKey(info.Key))
                        dict.Add(info.Key, info);
                }
            }

            // imagelist schema: 
            // [name]\t[ori_url]\t[thumbnail url]\t[total face]\t[face index]\t[age]\t[gender]\t[landmarks]\t[boundingbox]\t[beauty level]\t[beauty score]
            using (StreamReader pf = new StreamReader(pathImageList))
            {
                string line;
                while ((line = pf.ReadLine()) != null)
                {
                    var item = line.Split('\t');
                    string key = String.Format("{0}_{1}", item[0], item[4]);

                    if (dict.ContainsKey(key))
                    {
                        FaceInfo info = dict[key];
                        info.OriImgPath = item[1];
                        info.ThumbnailPath = item[2];
                        info.BeautyScoreDict.Add(labeler, Convert.ToSingle(item[10].Trim()));
                        info.BeautyLevelDict.Add(labeler, Convert.ToInt32(item[9].Trim()));

                        faceInfo.Add(info);
                    }
                }
            }

            return faceInfo;
        }

        public float CalFaceSDKFeatDist(FaceFeature queryFeat, FaceFeature trainFeat)
        {
            return _recognizerCnn.GetFaceFeatureDistance(queryFeat, trainFeat);
        }

        private string GenRootFeat(string pathImage, string component)
        {
            string rootFeat;

            if (pathImage.Contains("weibo"))
                rootFeat = @"D:\Work\FaceData\Face_weibo\feature\SDK_" + component;
            else if (pathImage.Contains("crawler"))
                rootFeat = @"D:\Work\FaceData\Face_crawler\feature\SDK_" + component;
            else if (pathImage.Contains("smth"))
                rootFeat = @"D:\Work\FaceData\Face_smth\feature\SDK_" + component;
            else if (pathImage.Contains("salog"))
                rootFeat = @"D:\Work\FaceData\Face_salog\feature\SDK_" + component;
            else if (pathImage.Contains("test"))
                rootFeat = @"D:\Work\FaceData\Face_test\feature\SDK_" + component;
            else if (pathImage.Contains("peculiar"))
                rootFeat = @"D:\Work\FaceData\Face_peculiar\feature\SDK_" + component;
            else if (pathImage.Contains("recruitment"))
                rootFeat = @"C:\Users\shuowan.FAREAST\Desktop\recruitment\feature\SDK_" + component;
            else
                throw (new Exception("Wrong pathImage " + pathImage));

            if (!Directory.Exists(rootFeat))
                Directory.CreateDirectory(rootFeat);

            return rootFeat;
        }
    }
}
