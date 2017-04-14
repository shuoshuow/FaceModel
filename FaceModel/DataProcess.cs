using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FaceSdk;
using System.Diagnostics;
using System.Drawing;

namespace FaceBeauty
{
    public class myReverserClass : IComparer
    {
        int IComparer.Compare(Object x, Object y)
        {
            if ((float)x < (float)y) return 1;
            if ((float)x > (float)y) return -1;
            return 0;
        }
    }

    class DataProcess
    {
        private static readonly DataProcess instance = new DataProcess();
        public static DataProcess Instance { get { return instance; } }

        private readonly FaceBasicFunc _basicFunc = FaceBasicFunc.Instance;
        private readonly Feature _featFunc = Feature.Instance;

        public DataProcess()
        {
            _basicFunc.Reload();
            _featFunc.Reload();
        }

        public void DedupFaces()
        {
            string labeler = "M80s";
            string pathImageList = @"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_M_80s_18K.tsv";
            string pathFeat = @"D:\Work\FaceData\FaceData_Train\feature\old\SDK_Face_M_18K_80s.dat";
            string pathDedupList = @"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_M_80s_18K_dedup.tsv";

            // Step 1: load features and scores
            List<FaceInfo> dataInfo = _featFunc.LoadFaceSDKFeat(pathFeat, pathImageList, labeler);


            // Step 2: calculate feature distance, if dist < 0.1 then the two face are considered duplicated
            var N = dataInfo.Count();
            var faceID = new int[N];
            for (int i = 0; i < N; i++)
            {
                faceID[i] = i;
            }

            for (int i = 0; i < N; i++)
            {
                for (int j = i + 1; j < N; j++)
                {
                    // current face is dup
                    if (faceID[j] != j)
                        continue;

                    if (_featFunc.CalFaceSDKFeatDist(dataInfo[i].FaceFeat, dataInfo[j].FaceFeat) < 0.1f)
                    {
                        faceID[j] = i;
                        dataInfo[i].BeautyScoreDict[labeler] = Math.Min(dataInfo[i].BeautyScoreDict[labeler], dataInfo[j].BeautyScoreDict[labeler]);
                        dataInfo[i].BeautyLevelDict[labeler] = Math.Min(dataInfo[i].BeautyLevelDict[labeler], dataInfo[j].BeautyLevelDict[labeler]);
                    }
                }
            }

            // Step 3: write to file
            StreamReader pf = new StreamReader(pathImageList);
            StreamWriter pfDedup = new StreamWriter(pathDedupList);
            for (int i = 0; i < N; i++)
            {
                // imagelist schema: 
                // [name]\t[ori_url]\t[thumbnail url]\t[total face]\t[face index]\t[age]\t[gender]\t[landmarks]\t[boundingbox]\t[beauty level]\t[beauty score]
                string line = pf.ReadLine();
                string text = string.Empty;

                if (faceID[i] == i)
                {
                    var item = line.Split('\t');
                    item[10] = string.Format("{0:0.00}", dataInfo[i].BeautyScoreDict[labeler]);
                    item[9] = string.Format("{0}", dataInfo[i].BeautyLevelDict[labeler]);

                    text = item[0];
                    for (int j = 1; j < item.Length; j++)
                        text += '\t' + item[j];

                    pfDedup.WriteLine(text);
                }
            }
            pf.Close();
            pfDedup.Close();
        }
        
        public void ImageSelectionByFaceDetection(string root)
        {
            //string path_image = Path.Combine(root, "Image", "raw");
            //string path_output = Path.Combine(root, "log", "FaceInfo.tsv");
            string path_image = Path.Combine(root, "Male");
            string path_output = Path.Combine(root, "log", "FaceInfo_male.tsv");

            if (!Directory.Exists(path_image))
                throw (new Exception("[FAIL] The image folder is not found: " + path_image));
            if (!Directory.Exists(Path.Combine(root, "log")))
                Directory.CreateDirectory(Path.Combine(root, "log"));

            string[] imagelist = Directory.GetFiles(path_image, "*.jpg");
            StreamWriter pf = new StreamWriter(path_output);

            int n = 0;
            string text;
            foreach (string imagePath in imagelist)
            {
                // output schema:
                // [name]\t[url]\t[face tag]\t[total face]\t[face index]\t[age]\t[gender]\t[landmarks]\t[bounding box]                
                try
                {
                    Console.WriteLine(string.Format("[{0}/{1}] is processing", n++, imagelist.Count()));

                    // face detection
                    List<FaceInfo> faceBasicInfo = _basicFunc.FaceDetection(imagePath, new string[]{"gender", "age", "pose"});
                    var N = faceBasicInfo.Count();

                    // if the image does not contain face
                    if (N == 0)
                    {
                        text = string.Format("{0}\t{1}\t", Path.GetFileNameWithoutExtension(imagePath), imagePath);
                        pf.WriteLine(text + "Invalid_noface");
                        continue;
                    }

                    // if the image contains face 
                    for(int i = 0; i < N; i++)                        
                    {
                        var face = faceBasicInfo[i];

                        text = string.Format("{0}\t{1}\t", Path.GetFileNameWithoutExtension(imagePath), imagePath);

                        // Case 1. if the face is too small
                        if (face.BoundingBox.Width < 80 || face.BoundingBox.Height < 80)                        
                            text += "Invalid_smallface\t";
                        // Case 2. if the person is a kid
                        else if (face.Age <= 6)                        
                            text += "Invalid_kids\t";
                        // Case 3. if the face is too side
                        else if (face.Pose.Yaw > 35 || face.Pose.Yaw < -35)
                            text += "Invalid_side\t";
                        // Default: valid face
                        else
                            text += "Valid\t";

                        text += string.Format("{0}\t{1}\t", N, i);
                        //text += String.Format("{0}\t{1}\t", face.Age, face.Gender.Gender);
                        text += String.Format("{0}\t{1}\t", face.Age, "Male");
                        for (int pt = 0; pt < face.Landmarks.Count; pt++)
                            text += string.Format("{0, 4:0.00},{1, 4:0.00} ", face.Landmarks[pt].X, face.Landmarks[pt].Y);
                        text = text.Trim();
                        text += string.Format("\t{0} {1} {2} {3}", face.BoundingBox.Left, face.BoundingBox.Top, face.BoundingBox.Width, face.BoundingBox.Height);
 
                        pf.WriteLine(text);
                    }
                }
                catch(Exception e)
                {
                    Console.WriteLine(e.ToString());
                }
            }
            pf.Close();
        }

        public void SplitFaceInfoFile(string root)
        {
            string path = Path.Combine(root, "log", "FaceInfo.tsv");
            StreamReader pf = new StreamReader(path);

            int nValid = 0, nNoFace = 0, nSmallFace = 0, nKid = 0, nSide = 0, nLabel = 0, nFemale = 0, nMale = 0;
            StreamWriter pfValid = new StreamWriter(Path.Combine(root, "log", "FaceInfo_Valid.tsv"));
            StreamWriter pfNoFace = new StreamWriter(Path.Combine(root, "log", "FaceInfo_Invalid_noface.tsv"));
            StreamWriter pfSmallFace = new StreamWriter(Path.Combine(root, "log", "FaceInfo_Invalid_smallface.tsv"));
            StreamWriter pfKid = new StreamWriter(Path.Combine(root, "log", "FaceInfo_Invalid_kids.tsv"));
            StreamWriter pfSide = new StreamWriter(Path.Combine(root, "log", "FaceInfo_Invalid_side.tsv"));
            StreamWriter pfLabel = new StreamWriter(Path.Combine(root, "log", "FaceInfo_Invalid_label.tsv"));
            StreamWriter pfFemale = new StreamWriter(Path.Combine(root, "log", "Female.tsv"));
            StreamWriter pfMale = new StreamWriter(Path.Combine(root, "log", "Male.tsv"));

            string line;
            while ((line = pf.ReadLine()) != null)
            {
                var item = line.Split('\t');
                switch (item[2])
                {
                    case "Valid":
                        pfValid.WriteLine(line);
                        if (item[6].Contains("Female"))
                        {
                            pfFemale.WriteLine(line);
                            nFemale++;
                        }
                        else if (item[6].Contains("Male"))
                        {
                            pfMale.WriteLine(line);
                            nMale++;
                        }
                        nValid++;
                        break;
                    case "Invalid_noface":
                        pfNoFace.WriteLine(line);
                        nNoFace++;
                        break;
                    case "Invalid_smallface":
                        pfSmallFace.WriteLine(line);
                        nSmallFace++;
                        break;
                    case "Invalid_kids":
                        pfKid.WriteLine(line);
                        nKid++;
                        break;
                    case "Invalid_side":
                        pfSide.WriteLine(line);
                        nSide++;
                        break;
                    case "Invalid_label":
                        pfLabel.WriteLine(line);
                        nLabel++;
                        break;
                    default:
                        throw (new Exception("Wrong face type"));
                }
            }
            pfValid.Close();
            pfNoFace.Close();
            pfSmallFace.Close();
            pfKid.Close();
            pfSide.Close();
            pfLabel.Close();
            pfFemale.Close();
            pfMale.Close();

            Console.WriteLine("Valid data: {0}", nValid);
            Console.WriteLine("Female data: {0}", nFemale);
            Console.WriteLine("Male_side data: {0}", nMale);
            Console.WriteLine("Invalid_noFace data: {0}", nNoFace);
            Console.WriteLine("Invalid_smallFace data: {0}", nSmallFace);
            Console.WriteLine("Invalid_kids data: {0}", nKid);
            Console.WriteLine("Invalid_side data: {0}", nSide);
        }

        public void ImageVisualization(string root, string type)
        {
            string path_image = Path.Combine(root, "Image", "raw");
            string path_output = Path.Combine(root, "Image", type);
            string path_log = Path.Combine(root, "log", String.Format("FaceInfo_{0}.tsv", type));

            if (!Directory.Exists(path_image))
                throw (new Exception("[FAIL] The image folder is not found: " + path_image));
            if (!Directory.Exists(path_output))
                Directory.CreateDirectory(path_output);

            StreamReader pf = new StreamReader(path_log);
            string line;
            while ((line = pf.ReadLine()) != null)
            {
                var item = line.Split('\t');

                if (File.Exists(Path.Combine(path_output, Path.GetFileName(item[0]))))
                    continue;

                Bitmap img = new Bitmap(item[1]);
                img.Save(Path.Combine(path_output, Path.GetFileName(item[0])));
            }
            pf.Close();
        }
        
        public void MapLabelResult()
        {
            string labeler = "F80sUS";
            string gender = "M";

            List<string> pathSet = new List<string> { @"D:\Work\FaceData\Face_crawler\log\FaceInfo_Valid.tsv",
                                                      @"D:\Work\FaceData\Face_weibo\log\FaceInfo_Valid.tsv",
                                                      @"D:\Work\FaceData\Face_tutu\log\FaceInfo_Valid.tsv",
                                                      @"D:\Work\FaceData\Face_smth\log\FaceInfo_Valid.tsv",
                                                      @"D:\Work\FaceData\Face_salog\log\FaceInfo_Valid.tsv",
                                                     };

            StreamReader pfLabel = new StreamReader(string.Format(@"D:\Work\FaceData\FaceData_Train\label_beauty_results\{0}\analysis_{1}_{2}.tsv", labeler, labeler, gender));
            StreamWriter pfOut = new StreamWriter(string.Format(@"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_{0}_{1}.tsv", labeler, gender));

            Dictionary<string, string> dict = new Dictionary<string, string>();
            string line;
            foreach (string path in pathSet)
            {
                StreamReader pfFaceInfo = new StreamReader(path);
                while ((line = pfFaceInfo.ReadLine()) != null)
                {
                    var item = line.Split('\t');
                    string key = string.Format("{0}_{1}", item[0], item[4]);
                    dict.Add(key, line);
                }
                pfFaceInfo.Close();
            }

            while ((line = pfLabel.ReadLine()) != null)
            {               
                var item = line.Split('\t');
                if (item[4] == "-1")
                    continue;
                string key = Path.GetFileNameWithoutExtension(item[0]);
                string value;
                if (!dict.TryGetValue(key, out value))
                    continue;

                // schema: [name]\t[ori_url]\t[thumbnail url]\t[total face]\t[face index]\t[age]\t[gender]\t[landmarks]\t[boundingbox]\t[beauty level]\t[beauty score]
                var faceitem = value.Split('\t');
                string thumbnailURL;
                if (item[0].StartsWith("."))
                    thumbnailURL = Path.Combine(@"D:\Work\FaceData", item[0].Substring(3));
                else
                    thumbnailURL = item[0];
                string text = string.Format("{0}\t{1}\t{2}", faceitem[0], faceitem[1], thumbnailURL);
                for (int i = 3; i < 9; i++)
                {
                    text += '\t' + faceitem[i].Trim();
                }
                text += '\t' + item[4] + '\t' + item[2];
                pfOut.WriteLine(text);
            }            
            pfLabel.Close();
            pfOut.Close();
        }

        public void TestGender(string root)
        {
            string line;

            StreamReader pfOld = new StreamReader(Path.Combine(root, @"log\FaceInfo_ori.tsv"));
            Dictionary<string, string> dict = new Dictionary<string, string>();
            while ((line = pfOld.ReadLine()) != null)
            {
                var item = line.Split('\t');

                if (item[2].Contains("Invalid"))
                    continue;

                dict.Add(string.Format("{0}_{1}", item[0], item[4]), item[6]);
            }

            StreamReader pf = new StreamReader(Path.Combine(root, @"log\FaceInfo.tsv"));
            int nF = 0, nM = 0, n = 0;
            float accF_old = 0f, accM_old = 0f;
            float accF_new = 0f, accM_new = 0f;
            while ((line = pf.ReadLine()) != null)
            {
                var item = line.Split('\t');

                if (item[2].Contains("Invalid"))
                    continue;

                string key = string.Format("{0}_{1}", item[0], item[4]);
                if (!dict.ContainsKey(key))
                    continue;

                // Extract face feature, assume that there is at least one face detected in image.                
                var faceInfo = new FaceInfo() {OriImgPath = item[1]};
                faceInfo.SetFaceRectLandmarks(item[8], item[7]);
                _basicFunc.FaceAttributePredict(faceInfo, new[] {"gender"});

                string genderOld;
                dict.TryGetValue(key, out genderOld);

                string gt = item[6];
                if (gt == "Female")
                {
                    nF++;
                    if (faceInfo.Gender.Gender.ToString() == gt) 
                        accF_new++;
                    if (genderOld == gt)
                        accF_old++;
                }
                else if (gt == "Male")
                {
                    nM++;
                    if (faceInfo.Gender.Gender.ToString() == gt) 
                        accM_new++;
                    if (genderOld == gt)
                        accM_old++;
                }

                if (n % 20 == 0)
                    Console.Write('\n');
                Console.Write(string.Format("{0} ", n++));

                //if(n==200)
                //    break;
            }

            Console.WriteLine("\n[Gender Prediction] Female Accuracy: Old model&old wrapper - {0:0.00%} | old model&new wrapper - {1:0.00%}", accF_old /nF, accF_new/nF);
            Console.WriteLine("[Gender Prediction] Male Accuracy: Old model&old wrapper - {0:0.00%} | old model&new wrapper - {1:0.00%}", accM_old/nM, accM_new/nM);

        }
        public void CombineFile(string root, string prefix)
        {
            //string root = @"D:\Work\salogging\label_gender\";
            //string prefix = "salog_label_gender";

            StreamWriter pf = new StreamWriter(Path.Combine(root, "label_gender", "total_label_gender.tsv"));            

            string[] femaleList = Directory.GetFiles(Path.Combine(root, "label_gender"), prefix + "_F_*_judgment.tsv");
            string[] maleList = Directory.GetFiles(Path.Combine(root, "label_gender"), prefix + "_M_*_judgment.tsv");

            for (int i = 0; i < femaleList.Length; i++)
            {
                StreamReader pf_label = new StreamReader(femaleList[i]);
                string line;
                while ((line = pf_label.ReadLine()) != null)
                    pf.WriteLine(line);
                pf_label.Close();
            }

            for (int i = 0; i < maleList.Length; i++)
            {
                StreamReader pf_label = new StreamReader(maleList[i]);
                string line;
                while ((line = pf_label.ReadLine()) != null)
                    pf.WriteLine(line);
                pf_label.Close();
            }

            pf.Close();
        }

        public void VerifyGender(string root)
        {
            //string root = @"D:\Work\FaceData\Face_crawler";            
            StreamReader pf_label = new StreamReader(Path.Combine(root, "label_gender", "total_label_gender.tsv"));
            StreamReader pf_faceInfo = new StreamReader(Path.Combine(root, "log", "FaceInfo.tsv"));
            StreamWriter pf_faceInfo_new = new StreamWriter(Path.Combine(root, "log", "FaceInfo_verified.tsv"));

            Dictionary<string, string> genderInfo = new Dictionary<string, string>();
            string line_label;
            string gender;
            string imageName;
            while ((line_label = pf_label.ReadLine()) != null)
            {
                imageName = Path.GetFileNameWithoutExtension(line_label.Split('\t')[0]);
                gender = line_label.Split('\t')[1].Trim();
                genderInfo.Add(imageName, gender);
            }
            
            string line;
            string text;
            while ((line = pf_faceInfo.ReadLine()) != null)
            {
                var item = line.Split('\t');

                if (item[2].StartsWith("Invalid"))
                {
                    pf_faceInfo_new.WriteLine(line);
                    continue;
                }

                imageName = item[0] + "_" + item[4];
                if(!genderInfo.TryGetValue(imageName, out gender))
                {
                    pf_faceInfo_new.WriteLine(line);
                    continue;
                }

                if (gender == "Invalid")
                    item[2] = "Invalid_label";
                else if (gender != item[6].Trim())
                    item[6] = gender;

                text = item[0];
                for (int i = 1; i < item.Count(); i++)
                    text += string.Format("\t{0}", item[i].Trim());

                pf_faceInfo_new.WriteLine(text);
            }

            pf_label.Close();
            pf_faceInfo.Close();
            pf_faceInfo_new.Close();
        }

        public void SplitImageSetByScore()
        {
            string root_output = @"D:\Work\FaceData\FaceData_Train\label_beauty_results\80sAmerican\SplitByScore\Male";
            StreamReader pf = new StreamReader(@"D:\Work\FaceData\FaceData_Train\label_beauty_results\80sAmerican\analysis_80s_M_American.tsv");

            string line;
            while ((line = pf.ReadLine()) != null)
            {
                line = line.Trim();
                var item = line.Split('\t');

                string pathImage = item[0];

                try
                {
                    if (!System.IO.Directory.Exists(Path.Combine(root_output, item[4])))
                        System.IO.Directory.CreateDirectory(Path.Combine(root_output, item[4]));

                    File.Copy(pathImage, Path.Combine(root_output, item[4], Path.GetFileName(pathImage)), true);
                }
                catch { ;}
            }

            pf.Close();
        }
        public void ProcessParticularList()
        {
            string pathOri = @"D:\Work\FaceData\Face_peculiar\log\labelFaceInfo_blacklist.tsv";
            string pathNew = @"D:\Work\FaceData\Face_peculiar\log\labelFaceInfo_blacklist_.tsv";

            StreamReader pfOri = new StreamReader(pathOri);
            StreamWriter pfNew = new StreamWriter(pathNew);
            string line;
            while((line = pfOri.ReadLine()) != null)
            {
                var item = line.Split('\t');
                item[2] = Path.Combine(@"D:\Work\FaceData\Face_peculiar\Image\Face", string.Format("{0}_{1}.bmp", item[0], item[4]));

                string text = item[0];
                for (int i = 1; i < item.Length; i++)
                    text += '\t' + item[i];
                pfNew.WriteLine(text);
            }
            pfOri.Close();
            pfNew.Close();
        }
        public void MergeImagelist()
        {
            List<string> allPaths = new List<string>
            {
                @"D:\Work\FaceData\Face_salog\log\Male.tsv", 
                @"D:\Work\FaceData\Face_smth\log\Male.tsv", 
                @"D:\Work\FaceData\Face_tutu\log\Male.tsv",
                @"D:\Work\FaceData\Face_crawler\log\residue_Male.tsv",
                @"D:\Work\FaceData\Face_weibo\log\residue_Male.tsv",
            };
            List<string> pathset = new List<string>();
            string path_output = Path.Combine(@"D:\Work\FaceData\FaceData_Train\log_trainlist", "trainlist_02_male.tsv");

            StreamWriter pf_output = new StreamWriter(path_output);

            List<string> content = new List<string>();
            foreach (string path in allPaths)
            {
                StreamReader pf = new StreamReader(path);

                string line;
                string text;
                while ((line = pf.ReadLine()) != null)
                {
                    var item = line.Split('\t');
                    text = String.Format("{0}\t{1}", item[0], Path.Combine(@"..\", path.Split('\\')[3], "Image", "Face", string.Format("{0}_{1}.bmp",item[0], item[4])));
                    for (int i = 2; i < item.Length; i++)
                        text += '\t' + item[i];

                    content.Add(text);
                }

                pf.Close();
            }

            Shuffle(content);
            foreach (string text in content)
                pf_output.WriteLine(text);

            pf_output.Close();
        }

        private void Shuffle<T>(IList<T> list)
        {
            Random rng = new Random();
            int n = list.Count;
            while (n > 1)
            {
                n--;
                int k = rng.Next(n + 1);
                T value = list[k];
                list[k] = list[n];
                list[n] = value;
            }
        }

        public void DataSplit()
        {
            // data split
            string[] faceInfo = File.ReadAllLines(@"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_M_80s_18K.tsv");
            StreamWriter pfTrain = new StreamWriter(@"D:\Work\FaceData\FaceData_Train\log\Train_F_salog.tsv");
            StreamWriter pfTest = new StreamWriter(@"D:\Work\FaceData\FaceData_Train\log\Test_M.tsv");

            for (int i = 0; i < faceInfo.Length; i++)
            {
                var item = faceInfo[i].Split('\t');
                if (item[1].Contains("salog"))
                {
                    pfTest.WriteLine(faceInfo[i]);
                }      
                else
                    pfTrain.WriteLine(faceInfo[i]);
            }
            pfTrain.Close();
            pfTest.Close();
        }

        public void Sync()
        {
            //sync
            string path_old = @"D:\Work\FaceData\Face_crawler\Image\combine.tsv";
            string path_new = @"D:\Work\FaceData\Face_crawler\log\FaceInfo_ori.tsv";

            StreamReader pf_old = new StreamReader(path_old);
            StreamReader pf_new = new StreamReader(path_new);
            StreamWriter pf = new StreamWriter(@"D:\Work\FaceData\Face_crawler\log\FaceInfo_.tsv");
            StreamWriter pf_residue = new StreamWriter(@"D:\Work\FaceData\Face_crawler\log\residue_FaceInfo_.tsv");

            Dictionary<string, string> info = new Dictionary<string, string>();
            string line;
            while ((line = pf_old.ReadLine()) != null)
            {
                info.Add(line.Split('\t')[0], line);
            }

            while ((line = pf_new.ReadLine()) != null)
            {
                var item = line.Split('\t');
                string value;

                if (!info.TryGetValue(item[0], out value))
                {
                    pf.WriteLine(line);
                    if (item[2] == "Valid")
                        pf_residue.WriteLine(line);
                }
                else
                {
                    if (item[2] == "Invalid_noface")
                    {
                        if (value.Split('\t')[3] == "Invalid_label")
                            pf.WriteLine(line);
                        else
                        {
                            string text = item[0] + '\t' + item[1] + "\tValid\t1\t0\t";
                            string bd = value.Split('\t')[5];
                            string str = string.Format("{0} {1} {2} {3}", bd.Split(',')[0], bd.Split(',')[1], bd.Split(',')[2], bd.Split(',')[3]);
                            text += String.Format("{0}\t{1}\t{2}\t{3}\t{4}", value.Split('\t')[2], value.Split('\t')[3], value.Split('\t')[4], str, -1);
                            pf.WriteLine(text);
                        }
                    }
                    else if (Convert.ToInt32(item[3]) > 1)
                    {
                        pf.WriteLine(line);
                        if (item[2] == "Valid")
                            pf_residue.WriteLine(line);
                    }
                    else
                    {
                        if (value.Split('\t')[3] == "Invalid_label")
                            item[2] = "Invalid_label";
                        else
                        {
                            if (item[2] != "Valid")
                                item[2] = "Valid";
                            if (item[6] != value.Split('\t')[3])
                                item[6] = value.Split('\t')[3];
                        }

                        string text = item[0];
                        for (int i = 1; i < item.Length; i++)
                            text += "\t" + item[i];
                        pf.WriteLine(text);
                    }
                }
            }

            pf_old.Close();
            pf_new.Close();
            pf.Close();
            pf_residue.Close();
        }
        public void SyncLabel()
        {
            //sync label_beauty results
            List<string> pathSet = new List<string> { @"D:\Work\FaceData\Face_weibo\log\FaceInfo_Valid.tsv", @"D:\Work\FaceData\Face_crawler\log\FaceInfo_Valid.tsv" };

            Dictionary<string, List<string>> info = new Dictionary<string, List<string>>();
            List<string> value;
            string line;

            foreach (string path in pathSet)
            {
                StreamReader pf = new StreamReader(path);
                while ((line = pf.ReadLine()) != null)
                {
                    if (info.TryGetValue(line.Split('\t')[0], out value))
                    {
                        value.Add(line);
                        info[line.Split('\t')[0]] = value;
                    }
                    else
                    {
                        value = new List<string>();
                        value.Add(line);
                        info.Add(line.Split('\t')[0], value);
                    }
                }
                pf.Close();
            }

            string[] labeler = new string[5] { "guoxia", "huimin", "jingddong", "pan", "ziqi" };
            //string[] labeler = new string[5] { "jing", "lang", "liuchang", "tangxi", "zhaoziyi" };

            for (int i = 0; i < labeler.Length; i++)
            {

                string root_ori = Path.Combine(@"D:\Work\FaceData\FaceData_Train\label_beauty_results\LabelResults_01\80label", labeler[i]);
                string root_new = Path.Combine(@"D:\Work\FaceData\FaceData_Train\label_beauty_results\80s", labeler[i]);
                if (!Directory.Exists(root_new))
                    Directory.CreateDirectory(root_new);

                var pathLabel = Directory.GetFiles(root_ori, "train_label_beauty_M_*_judgment.tsv");
                foreach (string path in pathLabel)
                {
                    StreamReader pf = new StreamReader(path);
                    StreamWriter pf_new = new StreamWriter(Path.Combine(root_new, Path.GetFileName(path)));

                    while ((line = pf.ReadLine()) != null)
                    {
                        string imageName = Path.GetFileNameWithoutExtension(line.Split('\t')[0]);
                        info.TryGetValue(imageName, out value);

                        if (imageName == "")
                            continue;

                        if (value.Count() != 1 || Convert.ToInt32(value[0].Split('\t')[3]) > 1)
                            continue;

                        string rootImage = value[0].Split('\t')[1];
                        string text;
                        if (rootImage.Contains(@"Face_weibo"))
                            text = string.Format("{0}_0.bmp\t{1}", Path.Combine(@"D:\Work\FaceData\Face_weibo\Image\Face\", imageName), line.Split('\t')[1]);
                        else if (rootImage.Contains(@"Face_crawler"))
                            text = string.Format("{0}_0.bmp\t{1}", Path.Combine(@"D:\Work\FaceData\Face_crawler\Image\Face\", imageName), line.Split('\t')[1]);
                        else
                            text = null;

                        pf_new.WriteLine(text);
                    }

                    pf.Close();
                    pf_new.Close();
                }
            }
        }

        public void SyncLabel2()
        {
            string[] labeler = new string[5] { "gianny", "k", "Lisa", "shelby", "vicsimno" };

            for (int i = 0; i < labeler.Length; i++)
            {

                string root_ori = Path.Combine(@"D:\Work\FaceData\FaceData_Train\label_beauty_results\LabelResults_01\80labelAmerican", labeler[i]);
                string root_new = Path.Combine(@"D:\Work\FaceData\FaceData_Train\label_beauty_results\80sAmerican", labeler[i]);
                if (!Directory.Exists(root_new))
                    Directory.CreateDirectory(root_new);

                var pathLabel = Directory.GetFiles(root_ori, "label_beauty_*_*_judgment.tsv");
                foreach (string path in pathLabel)
                {
                    StreamReader pf = new StreamReader(path);
                    StreamWriter pf_new = new StreamWriter(Path.Combine(root_new, Path.GetFileName(path)));

                    string line;
                    string text;
                    while ((line = pf.ReadLine()) != null)
                    {
                        var item = line.Split('\t');

                        if (item[0].Contains("Face_weibo"))
                            text = string.Format("{0}\t{1}", Path.Combine(@"D:\Work\FaceData\Face_weibo\Image\Face\", Path.GetFileName(item[0])), item[1]);
                        else if (item[0].Contains("Face_crawler"))
                            text = string.Format("{0}\t{1}", Path.Combine(@"D:\Work\FaceData\Face_crawler\Image\Face\", Path.GetFileName(item[0])), item[1]);
                        else
                            continue;

                        pf_new.WriteLine(text);
                    }

                    pf.Close();
                    pf_new.Close();
                }
            }
        }

        public void ProcessJapData()
        {
            StreamReader pfMap = new StreamReader(@"D:\Work\FaceData\Face_Jan\log\MappingFile.txt");
            Dictionary<string, string> dictURL = new Dictionary<string, string>();
            string line;
            while ((line = pfMap.ReadLine()) != null)
            {
                dictURL.Add(line.Split('\t')[0], line.Split('\t')[1]);
            }
            pfMap.Close();

            StreamReader pfList = new StreamReader(@"D:\Work\FaceData\Face_Jan\log\Avatar Results Processed.tsv");
            Dictionary<string, string> dictStar = new Dictionary<string, string>();
            while ((line = pfList.ReadLine()) != null)
            {
                string url = line.Split('\t')[1];
                string name = line.Split('\t')[0];

                if (dictURL.ContainsKey(url))
                {
                    if (dictStar.ContainsKey(dictURL[url]))
                        dictStar[dictURL[url]] = null;
                    else
                        dictStar.Add(dictURL[url], name);
                }
            }
            pfList.Close();

            StreamReader pfScore = new StreamReader(@"D:\Work\FaceData\Face_Jan\log\howprettyareyou-jaJP.txt");
            Dictionary<string, string> dictScore = new Dictionary<string, string>();
            while ((line = pfScore.ReadLine()) != null)
            {
                string text = line.Split('\t')[4];
                if (line.Split('\t')[1] == "男")
                    text += " Male";
                else
                    text += " Female";

                dictScore.Add(line.Split('\t')[3], text);
            }

            StreamReader pfInfo = new StreamReader(@"D:\Work\FaceData\Face_Jan\log\FaceInfo.tsv");
            StreamWriter pfOut = new StreamWriter(@"D:\Work\FaceData\Face_Jan\log\labelFaceInfo_Jap.tsv");
            while ((line = pfInfo.ReadLine()) != null)
            {
                var item = line.Split('\t');
                string path = item[1];

                if ((item[2] == "Valid" || item[2] == "Invalid_kids") && Convert.ToInt32(item[3]) == 1)
                {
                    if (dictStar[path] != null)
                    {
                        item[2] = dictStar[path];
                        item[6] = dictScore[dictStar[path]].Split(' ')[1];

                        string text = null;
                        foreach (var it in item)
                            text += it + '\t';
                        float score = Convert.ToSingle(dictScore[dictStar[path]].Split(' ')[0]);
                        int level = Math.Min(5, (int)Math.Floor((Convert.ToDecimal(score) - 1) / 0.8m) + 1);
                        text += string.Format("{0}\t{1:0.00}", level, score);
                        pfOut.WriteLine(text.Trim());
                    }
                }
            }
            pfOut.Close();
            pfInfo.Close();
        }

        public void SelectTestData()
        {
            string[] allLabelers = { "F80s", "F90s", "M80s", "M90s" };
            string gender = "F";

            StreamReader pfLabel;
            StreamWriter pfTrain, pfTest;
            string line;
            Dictionary<string, Dictionary<string, float>> dict = new Dictionary<string, Dictionary<string, float>>();

            foreach (var labeler in allLabelers)
            {
                pfLabel = new StreamReader(string.Format(@"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_{0}_{1}.tsv", labeler, gender));
                while ((line = pfLabel.ReadLine()) != null)
                {
                    var item = line.Split('\t');
                    string key = string.Format("{0}_{1}", item[0], item[4]);
                    Dictionary<string, float> beautyScore;
                    if (dict.TryGetValue(key, out beautyScore))
                    {
                        beautyScore.Add(labeler, Convert.ToSingle(item[10]));
                        dict[key] = beautyScore;
                    }
                    else
                    {
                        beautyScore = new Dictionary<string, float>();
                        beautyScore.Add(labeler, Convert.ToSingle(item[10]));
                        dict.Add(key, beautyScore);
                    }
                }
                pfLabel.Close();
            }

            int numPerLabeler = 100;
            int[] cnt = new int[5];
            for (int i = 0; i < 5; i++)
                cnt[i] = numPerLabeler;

            List<string> selected = new List<string>(numPerLabeler * allLabelers.Length);

            int n = 0;
            foreach (var key in dict.Keys)
            {
                bool isSelected = true;
                foreach (var labeler in allLabelers)
                {
                    if (!dict[key].ContainsKey(labeler))
                    {
                        isSelected = false;
                        break;
                    }
                }

                if (!isSelected)
                    continue;

                n++;
                float scoreF80s = dict[key]["F80s"];
                int levelF80s = ScoreToLevel(scoreF80s);
                if (cnt[levelF80s - 1] > 0)
                {
                    cnt[levelF80s - 1]--;
                    selected.Add(key);
                }
            }

            // write train list to file
            foreach (var labeler in allLabelers)
            {
                pfLabel = new StreamReader(string.Format(@"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_{0}_{1}.tsv", labeler, gender));
                pfTrain = new StreamWriter(string.Format(@"D:\Work\FaceData\FaceData_Train\log\Train_{0}_{1}.tsv", labeler, gender));

                while ((line = pfLabel.ReadLine()) != null)
                {
                    var item = line.Split('\t');
                    string key = string.Format("{0}_{1}", item[0], item[4]);

                    if (!selected.Contains(key))
                        pfTrain.WriteLine(line);
                }
                pfTrain.Close();
                pfLabel.Close();
            }

            // write test list to file
            pfLabel = new StreamReader(string.Format(@"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_{0}_{1}.tsv", allLabelers[0], gender));
            pfTest = new StreamWriter(string.Format(@"D:\Work\FaceData\FaceData_Train\log\Test_{0}.tsv", gender));

            while ((line = pfLabel.ReadLine()) != null)
            {
                var item = line.Split('\t');
                string key = string.Format("{0}_{1}", item[0], item[4]);

                if (selected.Contains(key))
                {
                    string text = null;
                    for (int i = 0; i <= 8; i++)
                        text += item[i] + '\t';
                    string score = null;
                    foreach (var labeler in allLabelers)
                        score += string.Format("{0}:{1:0.00} ", labeler, dict[key][labeler]);
                    text += score.Trim();
                    pfTest.WriteLine(text);
                }
            }
            pfTest.Close();
            pfLabel.Close();


        }

        static private int ScoreToLevel(float score)
        {
            return Math.Min(5, (int)Math.Floor((Convert.ToDecimal(score) - 1) / 0.8m) + 1);
        }

        public void FeatExtraction()
        {
            Feature FEAT = Feature.Instance;
            FEAT.Reload();

            // extrace feature
            string[] allLabelers = { "M90s" };
            string[] allGenders = { "F", "M" };
            foreach (var labeler in allLabelers)
            {
                foreach (var gender in allGenders)
                {
                    StreamReader pfQueryList = new StreamReader(string.Format(@"D:\Work\FaceData\FaceData_Train\log\Train_{0}_{1}.tsv", labeler, gender));
                    string pathQueryFeatList = string.Format(@"D:\Work\FaceData\FaceData_Train\feature\SDK_Face_{0}_{1}.dat", labeler, gender);
                    BinaryWriter writer = new BinaryWriter(File.Open(pathQueryFeatList, FileMode.Create));

                    List<FaceInfo> faceInfo = FEAT.LoadFaceSDKFeat(string.Format(@"D:\Work\FaceData\FaceData_Train\feature\new\SDK_Face_{0}_{1}.dat", labeler, gender));
                    var dict = new Dictionary<string, FaceInfo>();

                    foreach (var info in faceInfo)
                    {
                        if (!dict.ContainsKey(info.Key))
                            dict.Add(info.Key, info);
                    }

                    int n = 0;
                    string line;
                    while ((line = pfQueryList.ReadLine()) != null)
                    {
                        Console.Write("{0} ", n);
                        if (n != 0 && n % 10 == 0)
                            Console.Write('\n');
                        n++;

                        // imagelist schema:
                        // [image name]\t[original image path]\t[thumbnail image path]\t[total face number]\t[face index]\t[age]\t[gender]\t[landmark]\t[bounding box]\t[beauty level]
                        var item = line.Split('\t');

                        string key = String.Format("{0}_{1}", item[0], item[4]);

                        FaceFeature feat;
                        if (dict.ContainsKey(key))
                        {
                            feat = dict[key].FaceFeat;
                        }
                        else
                        {
                            // set face basic info
                            var tmpFaceInfo = new FaceInfo();
                            tmpFaceInfo.SetFaceRectLandmarks(item[8].Trim(), item[7].Trim());

                            // extrac feature
                            string pathImage = item[1];
                            var colorImage = ImageUtility.LoadImageFromBitmapAsRgb24(new Bitmap(pathImage));
                            feat = FEAT.ExtractFaceSDKFeat(colorImage, tmpFaceInfo.Landmarks, "Face");
                        }

                        // write to file                
                        writer.Write(String.Format("{0}_{1}", item[0], item[4]));
                        writer.Write(feat.Vector);
                    }
                    writer.Close();
                }
            }
        }

        public void PerfTest()
        {
            string imgPath = @"D:\Work\Code\GuessRelationUX\TestData\raw\20100417135258.jpg";

            Bitmap img = new Bitmap(imgPath);
            FaceSdk.IImage colorImage = ImageUtility.LoadImageFromBitmapAsRgb24(img);

            int N = 100;
            double avgDetTime = 0.0;
            double avgFeatTime = 0.0;
            for (int i = 0; i < N; i++)
            {
                var watch = Stopwatch.StartNew();
                List<FaceInfo> faceInfo = _basicFunc.FaceDetection(imgPath);
                watch.Stop();
                avgDetTime += watch.ElapsedMilliseconds;

                watch = Stopwatch.StartNew();
                foreach (var info in faceInfo)
                {
                    FaceFeature feat = _featFunc.ExtractFaceSDKFeat(colorImage, info.Landmarks);
                }
                watch.Stop();
                avgFeatTime += watch.ElapsedMilliseconds;
            }

            Console.WriteLine("Avg Detection Time: {0:0.00}ms", avgDetTime/N);
            Console.WriteLine("Avg Feature Extraction Time: {0:0.00}ms", avgFeatTime / N);
        }

        public void UpdateTrainTest()
        {
            string labeler = "F80sUS";
            string gender = "M";

            StreamReader pfTest = new StreamReader(string.Format(@"D:\Work\FaceData\FaceData_Train\log\Test_{0}.tsv", gender));
            StreamReader pfLabel = new StreamReader(string.Format(@"D:\Work\FaceData\FaceData_Train\log\labelFaceInfo_{0}_{1}.tsv", labeler, gender));
            StreamWriter pfTrainNew = new StreamWriter(string.Format(@"D:\Work\FaceData\FaceData_Train\log\Train_{0}_{1}.tsv", labeler, gender));
            StreamWriter pfTestNew = new StreamWriter(string.Format(@"D:\Work\FaceData\FaceData_Train\log\Test_{0}_new.tsv", gender));

            Dictionary<string, string> dict = new Dictionary<string, string>();
            string line;
            while ((line = pfLabel.ReadLine()) != null)
            {
                var item = line.Split('\t');
                string key = string.Format("{0}_{1}", item[0], item[4]);
                dict.Add(key, line);
            }
            pfLabel.Close();

            while ((line = pfTest.ReadLine()) != null)
            {
                var item = line.Split('\t');
                string key = string.Format("{0}_{1}", item[0], item[4]);

                string text;
                float s = -1;
                if (dict.TryGetValue(key, out text))
                    s = Convert.ToSingle(text.Split('\t')[10]);

                string[] score = item[9].Split(' ');
                Dictionary<string, float> scoredict = new Dictionary<string, float>();
                for (int i = 0; i < score.Length; i++)
                {
                    scoredict.Add(score[i].Split(':')[0], Convert.ToSingle(score[i].Split(':')[1]));
                }
                if(scoredict.ContainsKey(labeler))
                    scoredict[labeler] = s;
                else
                    scoredict.Add(labeler, s);

                item[9] = "";
                foreach(var lab in scoredict.Keys)
                {
                    item[9] += string.Format("{0}:{1:0.00} ", lab, scoredict[lab]);
                }
                item[9] = item[9].Trim();

                line = "";
                for (int i = 0; i < item.Length; i++)
                {
                    line += item[i] + '\t';
                }
                pfTestNew.WriteLine(line.Trim());

                dict.Remove(key);
            }
            pfTestNew.Close();

            foreach (var text in dict.Values)
                pfTrainNew.WriteLine(text);
            pfTrainNew.Close();
        }

        public void SampleTestList()
        {
            var Gender = "Male";
            string path01, path02;
            if (Gender == "Male")
            {
                path01 = @"D:\Work\FaceData\FaceData_Train\log\Test_M.tsv";
                path02 = @"D:\Work\FaceData\FaceData_Train\log_trainlist\testlist_newTask_M.tsv";
            }
            else
            {
                path01 = @"D:\Work\FaceData\FaceData_Train\log\Test_F.tsv";
                path02 = @"D:\Work\FaceData\FaceData_Train\log_trainlist\testlist_newTask_F.tsv";
            }

            using (var pf01 = new StreamReader(path01))
            using (var pfOut = new StreamWriter(path02))
            {

                string line;
                while ((line = pf01.ReadLine()) != null)
                {
                    var items = line.Split('\t');

                    var tmp = items[2];
                    items[2] = Path.Combine(@"D:\Work\FaceData\FaceData_Train\Face_newTask", Path.GetFileName(items[2]));
                    if (!File.Exists(items[2]))
                    {
                        var img = new Bitmap(tmp);
                        img.Save(items[2]);
                    }

                    var text = items[0];
                    for (int j = 1; j <= 8; j++)
                        text += '\t' + items[j];
                    pfOut.WriteLine(text);
                }
            }
        }

        public void SampleTrainList()
        {
            Dictionary<string, List<string>> data = new Dictionary<string, List<string>>();
            List<string> dataList = new List<string>();

            var Gender = "Male";
            string path01, path02, path03;
            Dictionary<string, int> sampleNum;
            if (Gender == "Male")
            {
                path01 = @"D:\Work\FaceData\FaceData_Train\log\Train_F80s_M.tsv";
                path02 = @"D:\Work\FaceData\Face_crawler\Image\black\log\Male.tsv";
                path03 = @"D:\Work\FaceData\FaceData_Train\log_trainlist\trainlist_newTask_M.tsv";
                sampleNum = new Dictionary<string, int>()
                {
                    {"1", 950},
                    {"2", 950},
                    {"3", 834},
                    {"4", 850},
                    {"5", 830},
                };
            }
            else
            {
                path01 = @"D:\Work\FaceData\FaceData_Train\log\Train_F80s_F.tsv";
                path02 = @"D:\Work\FaceData\Face_crawler\Image\black\log\Female.tsv";
                path03 = @"D:\Work\FaceData\FaceData_Train\log_trainlist\trainlist_newTask_F.tsv";
                sampleNum = new Dictionary<string, int>()
                {
                    {"1", 950},
                    {"2", 950},
                    {"3", 823},
                    {"4", 822},
                    {"5", 850},
                };
            }

            using (var pf01 = new StreamReader(path01))
            using (var pf02 = new StreamReader(path02))
            using (var pfOut = new StreamWriter(path03))
            {
                
                string line;
                while ((line = pf01.ReadLine()) != null)
                {
                    var items = line.Split('\t');
                    var key = items[9];

                    if (data.ContainsKey(key))
                    {
                        var value = data[key];
                        value.Add(line);
                        data[key] = value;
                    }
                    else
                    {
                        var value = new List<string>();
                        value.Add(line);
                        data.Add(key, value);
                    }
                }

                for (int i = 1; i <= 5; i++)
                {
                    var num = sampleNum[i.ToString()];
                    foreach (var str in data[i.ToString()].GetRange(0, num))
                    {
                        var items = str.Split('\t');

                        var tmp = items[2];
                        items[2] = Path.Combine(@"D:\Work\FaceData\FaceData_Train\Face_newTask\",
                            Path.GetFileName(items[2]));
                        if (!File.Exists(items[2]))
                        {
                            var img = new Bitmap(tmp);
                            img.Save(items[2]);
                        }

                        var text = items[0];
                        for (int j = 1; j <= 8; j++)
                            text += '\t' + items[j];
                        dataList.Add(text);
                    }
                }

                while ((line = pf02.ReadLine()) != null)
                {
                    var items = line.Split('\t');
                    var tmp = Path.Combine(@"D:\Work\FaceData\Face_crawler\Image\black\Face", string.Format("{0}_{1}.bmp", items[0], items[4]));
                    items[2] = Path.Combine(@"D:\Work\FaceData\FaceData_Train\Face_newTask\",
                            string.Format("{0}_{1}.bmp", items[0], items[4]));
                    if (!File.Exists(items[2]))
                    {
                        var img = new Bitmap(tmp);
                        img.Save(items[2]);
                    }

                    var text = items[0];
                    for (int j = 1; j <= 8; j++)
                        text += '\t' + items[j];
                    dataList.Add(text);
                }

                Shuffle<string>(dataList);

                foreach (var str in dataList)
                {
                    pfOut.WriteLine(str);
                }
            }
        }
    }
}
