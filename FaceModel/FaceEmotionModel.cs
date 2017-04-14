using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.ProjectOxford.Common;
using Microsoft.ProjectOxford.Emotion;
using Microsoft.ProjectOxford.Emotion.Contract;

namespace FaceModel
{    
    class FaceEmotionModel
    {
        private static readonly FaceEmotionModel instance = new FaceEmotionModel();
        public static FaceEmotionModel Instance { get { return instance; } }

        private Recognizer _recognizer = null;

        public static readonly Dictionary<string, int> EmotionCate = new Dictionary<string, int> { 
                                                                                                    {"Neutral", 0}, 
                                                                                                    {"Happiness", 1},
                                                                                                    {"Surprise", 2},
                                                                                                    {"Sadness", 3},
                                                                                                    {"Anger", 4},
                                                                                                    {"Contempt", 5},
                                                                                                    {"Disgust", 6},
                                                                                                    {"Fear", 7},
                                                                                                };
        public void Reload()
        {
            try
            {
                RecognizerModel model = new RecognizerModel(@".\Models\emotion_model.txt");
                _recognizer = new Recognizer(model);
            }
            catch (Exception e)
            {
                Trace.TraceError("Error loading emotion model: {0}", e.ToString());
                this._recognizer = null;
            }
        }

        public Emotion[] PredictCNN(string path, Microsoft.ProjectOxford.Common.Rectangle[] faceRects)
        {
            Emotion[] emotions = null;

            using (Stream stream = new FileStream(path, FileMode.Open, FileAccess.Read))
            {
                using (var image = new GrayscaleImage())
                {
                    image.Load(stream);

                    emotions = _recognizer.RecognizeImage(image, faceRects);
                }
            }

            return emotions;
        }

        public Emotion PredictCNN(Stream stream, Microsoft.ProjectOxford.Common.Rectangle faceRect)
        {
            Microsoft.ProjectOxford.Common.Rectangle[] faceRects = new Microsoft.ProjectOxford.Common.Rectangle[1];
            faceRects[0] = faceRect;

            try
            {
                Emotion[] emotions = _recognizer.RecognizeImage(stream, faceRects);
                return emotions[0];
            }
            catch (Exception)
            {
                return null;
            }

            
        }
    }
}
