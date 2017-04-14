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

namespace FaceModel
{
    class FaceBasicFunc
    {
        private static readonly FaceBasicFunc instance = new FaceBasicFunc();
        private FaceDetectionJDA _detectorJda = null;
        private FaceAttributeAgeCNN _attributeAge = null;
        private FaceAttributePose _attributePose = null;
        private FaceAttributeGenderCNN _attributeGender = null;
        private FaceAttributeFacialHairCNN _attributeFacialHair = null;
        private FaceAttributeGlassesCNN _attributeGlasses = null;
        private FaceRecognitionCNN _recognizerCnn = null;  

        public static FaceBasicFunc Instance { get { return instance; } }
        
        public void Reload()
        {
            try
            {
                _detectorJda = new FaceDetectionJDA(new Model(@".\Models\ProductCascadeJDA27ptsWithLbf.mdl"));
                _attributeAge = new FaceAttributeAgeCNN(new Model(@".\Models\ProductAttributeAgeCnn.mdl"));
                _attributePose = new FaceAttributePose(new Model(@".\Models\ProductAttributePose27Pts.mdl"));
                _attributeGender = new FaceAttributeGenderCNN(new Model(@".\Models\ProductAttributeGenderCnn.mdl"));
                _attributeFacialHair = new FaceAttributeFacialHairCNN(new Model(@".\Models\ProductAttributeFacialHairCnn.mdl"));
                _attributeGlasses = new FaceAttributeGlassesCNN(new Model(@".\Models\ProductAttributeGlassesCnn.mdl"));
                _recognizerCnn = new FaceRecognitionCNN(new Model(@".\Models\ProductRecognitionCnn27pts.mdl"));
            }
            catch (Exception e)
            {
                Trace.TraceError("Error loading attribute model: {0}", e.ToString());
                this._detectorJda = null;
                this._attributeAge = null;
                this._attributePose = null;
                this._attributeGender = null;
                this._attributeFacialHair = null;
                this._attributeGlasses = null;
                this._recognizerCnn = null;
            }
        }

        public List<FaceInfo> FaceDetection(string path, string[] attrOpts = null)
        { 
            FaceSdk.IImage grayImage = ImageUtility.LoadImageFromFileAsGray(path);
            int imgArea = grayImage.Width * grayImage.Height;

            var faces = _detectorJda.DetectAndAlign(grayImage, 24, 1024);

            List<FaceInfo> faceBasic = new List<FaceInfo>(faces.Count);
            foreach (var face in faces)
            {
                var info = new FaceInfo()
                {
                    OriImgPath = path,
                    BoundingBox = face.FaceRect,
                    Landmarks = face.Landmarks,
                    Ratio = (float)face.FaceRect.Area / (float)imgArea,
                    Pose = _attributePose.AnalyzePose(grayImage, face.Landmarks),
                };

                faceBasic.Add(info);
            }

            if(attrOpts == null)
                return faceBasic;

            FaceSdk.IImage colorImage = ImageUtility.LoadImageFromBitmapAsRgb24(new Bitmap(path));
            foreach (var face in faceBasic)
            {
                face.FaceFeat = FeatureExtraction(colorImage, face.Landmarks, attrOpts);
                FaceAttributePredict(face, attrOpts);
            }

            return faceBasic;
        }

        public void FaceAttributePredict(FaceInfo face, string[] attrOpts = null)
        {
            if (attrOpts == null || face.Landmarks == null)
                return;

            if (face.FaceFeat == null)
            {
                FaceSdk.IImage colorImage = ImageUtility.LoadImageFromBitmapAsRgb24(new Bitmap(face.OriImgPath));
                face.FaceFeat = FeatureExtraction(colorImage, face.Landmarks, attrOpts);
            }

            foreach (var opt in attrOpts)
            {
                switch (opt.ToLower())
                {
                    case "gender":
                        face.Gender = _attributeGender.Analyze(face.FaceFeat);
                        break;
                    case "age":
                        face.Age = _attributeAge.Analyze(face.FaceFeat);
                        break;
                    case "facialhair":
                        // Facial hair: moustache/beard/sideburns: facial hair on upper lip/chin/cheeks. This value is in range [0, 1].
                        // [0, 0.25]   indicates no moustache/beard/sideburns.
                        // [0.25, 0.5] indicates stubble moustache/beard/sideburns.
                        // [0.5, 0.75] indicates medium moustache/beard/sideburns.
                        // [0.75, 1]   indicates long moustache/beard/sideburns.
                        face.FacialHair = _attributeFacialHair.Analyze(face.FaceFeat);
                        break;
                    case "glass":
                        face.Glasses = _attributeGlasses.Analyze(face.FaceFeat);
                        break;
                }
            }
        }
        public FaceFeatureCNN FeatureExtraction(FaceSdk.IImage colorImage, FaceSdk.FaceLandmarks landmarks, string[] attrOpts = null)
        {
            if (attrOpts == null)
                return null;

            var paras = new List<FaceRecognitionCNN.Usage>();
            foreach (var opt in attrOpts)
            {
                switch (opt.ToLower())
                {
                    case "gender":
                        paras.Add(FaceRecognitionCNN.Usage.Gender);
                        break;
                    case "age":
                        paras.Add(FaceRecognitionCNN.Usage.Age);
                        break;
                    case "beauty":
                        paras.Add(FaceRecognitionCNN.Usage.BeautyIndex);
                        break;
                    case "facialhair":
                        paras.Add(FaceRecognitionCNN.Usage.FacialHair);
                        break;
                    case "glass":
                        paras.Add(FaceRecognitionCNN.Usage.Glasses);
                        break;
                }
            }

            FaceFeatureCNN faceFeat = _recognizerCnn.ExtractFaceFeature(colorImage, landmarks, paras);
            
            return faceFeat;
        }
    }
}
