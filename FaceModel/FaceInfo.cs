using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FaceSdk;
using Microsoft.ProjectOxford.Emotion.Contract;

namespace FaceModel
{
    class FaceInfo
    {
        public string Key { get; set; }
        public string OriImgPath { get; set; }
        public string ThumbnailPath { get; set; }
        public FaceSdk.Rectangle BoundingBox { get; set; }
        public FaceSdk.FaceLandmarks Landmarks { get; set; }
        public float Age { get; set; }
        public FaceSdk.FaceGenderResult Gender { get; set; }
        public FaceSdk.FacePose Pose { get; set; }
        public float Ratio { get; set; }
        public FaceFeatureCNN FaceFeat { get; set; }
        public Emotion EmotionScore { get; set; }
        public string DomiEmotion { get; set; }
        public float DomiEmotionScore { get; set; }
        public FaceGlassesResult Glasses { get; set; }
        public FacialHairResult FacialHair { get; set; }
        public Dictionary<string, float> BeautyScoreDict { get; set; }
        public Dictionary<string, int> BeautyLevelDict { get; set; }
        public float[] FacialFeature { get; set; }

        public FaceInfo()
        {
            Key = string.Empty;
            OriImgPath = string.Empty;
            ThumbnailPath = string.Empty;
            Age = 0f;
            Ratio = 0f;
            FaceFeat = null;
            EmotionScore = null;
            DomiEmotionScore = 0f;
            DomiEmotion = string.Empty;
            BeautyScoreDict = null;
            BeautyLevelDict = null;
            FacialFeature = null;
        }

        public void SetFaceRectLandmarks(string boundingBox, string landmarks)
        {
            var item = boundingBox.Split(' ');
            if (item.Count() < 4)
                item = boundingBox.Split(',');
            this.BoundingBox = new FaceSdk.Rectangle((int)Convert.ToSingle(item[0]), (int)Convert.ToSingle(item[1]), (int)Convert.ToSingle(item[2]), (int)Convert.ToSingle(item[3]));

            item = landmarks.Split(' ');
            FaceSdk.PointF[] points = new FaceSdk.PointF[item.Length];
            for (int i = 0; i < item.Length; i++)
            {
                points[i].X = Convert.ToSingle(item[i].Split(',')[0]);
                points[i].Y = Convert.ToSingle(item[i].Split(',')[1]);
            }
            this.Landmarks = new FaceSdk.FaceLandmarks
            {
                Points = points,
                LandmarkType = FaceLandmarkType.Landmark27Points
            };

            //this.landmarks.Name = "FaceLandmark27Points";
        }
    }
}
