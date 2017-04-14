using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FaceModel
{
    class FaceBasicTests
    {
        private readonly FaceBasicFunc _basicFunc = FaceBasicFunc.Instance;

        public FaceBasicTests()
        {
            _basicFunc.Reload();
        }

        public void TestFacialHair(string path)
        {
            var faceInfo = _basicFunc.FaceDetection(path, new string[] {"facialhair"});
            if (faceInfo.Count == 0)
                Console.WriteLine("No Face");
            else
            {
                Console.WriteLine(string.Format("Moustache:{0}", faceInfo[0].FacialHair.Moustache));
                Console.WriteLine(string.Format("Beard:{0}", faceInfo[0].FacialHair.Beard));
                Console.WriteLine(string.Format("Sideburns:{0}", faceInfo[0].FacialHair.Sideburns));
            }
        }

        public void TestGlasses(string path)
        {
            var faceInfo = _basicFunc.FaceDetection(path, new string[] { "glass" });
            if (faceInfo.Count == 0)
                Console.WriteLine("No Face");
            else 
                Console.WriteLine(string.Format("GlassType:{0}, Confidence:{1}", faceInfo[0].Glasses.Glasses, faceInfo[0].Glasses.Confidence));
        }
    }
}
