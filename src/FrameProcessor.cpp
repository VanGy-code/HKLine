//
// Created by vangy on 2021/10/24.
//

#include "FrameProcessor.h"

std::time_t FrameProcessor::getTimeStamp() {
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());//获取当前时间点
    std::time_t timestamp =  tp.time_since_epoch().count(); //计算距离1970-1-1,00:00的时间长度
    return timestamp;
}


void FrameProcessor::flipRight(cv::Mat &matSrc) {
    cv::Mat dst;
    transpose(matSrc, dst);
    flip(dst, matSrc, 1);
}

int FrameProcessor::combine(std::vector<cv::Mat>& frameBucket, bool softCombine, const std::string savePath, const std::string saveType) {
    /**
     * @description:
     */
    if(frameBucket.empty()) return -1;

    std::time_t timestamp;
    std::string filepath;

    if(softCombine){
        flipRight(frameBucket[0]);
        cv::Mat result = frameBucket[0];
        for (int i = 1; i < frameBucket.size(); i++) {
            // TODO: test LineCamera
            flipRight(frameBucket[i]);
            result = orbCombine(result, frameBucket[i]);
        }

        timestamp = getTimeStamp();
        filepath = std::to_string(timestamp);
        filepath = savePath + filepath + saveType;
        cv::imwrite(filepath, result);
        std::cout << "拼接完成" << std::endl;
        return 0;
    }
    else{
        // TODO: Force combine
        int height = 120 * frameBucket.size();
        int width = 4096;
        cv::Mat result = cv::Mat(height, width,CV_8UC3, cv::Scalar::all(0));
        int current_y = 0;
        for (cv::Mat frame : frameBucket){
            cv::Mat ROI = result(cv::Rect(0, current_y, width, frame.rows));
            current_y += 120;
            frame.copyTo(ROI);
        }
        cv::imwrite("output/result.bmp", result);
        std::cout << "拼接完成" << std::endl;
        return 0;
    }
}

std::vector<cv::Mat> FrameProcessor::readTestImage(const std::string pathPattern) {
    std::vector<cv::Mat> images;
    std::vector<cv::String> imageList;

    // glob by order
    cv::glob(pathPattern, imageList, false);
    sort(imageList.begin(), imageList.end(),
         [](const std::string& a, const std::string& b) {return std::atoi(a.c_str()) < std::atoi(b.c_str()); });

    for(const std::string& img_name : imageList)
    {
        cv::Mat src = cv::imread(img_name);
        if (src.type() == CV_8UC3)//彩色
        {
            //判断是否正确读入图片
            if (src.empty()) {
                std::cout << "could not load image..." << std::endl;
                break;
            }
            images.push_back(src);
        }
    }
    return images;
}

void FrameProcessor::CalcCorners(const cv::Mat &H, const cv::Mat &src) {
    double v2[] = { 0, 0, 1 };//左上角
    double v1[3]; //变换后的坐标值
    cv::Mat V2 = cv::Mat(3, 1, CV_64FC1, v2);  //列向量
    cv::Mat V1 = cv::Mat(3, 1, CV_64FC1, v1);  //列向量
    V1 = H * V2;

    //左上角(0,0,1)
//    std::cout << "V2: " << V2 << std::endl;
//    std::cout << "V1: " << V1 << std::endl;
    corners.left_top.x = v1[0] / v1[2];
    corners.left_top.y = v1[1] / v1[2];

    //左下角(0,src.rows,1)
    v2[0] = 0;
    v2[1] = src.rows;
    v2[2] = 1;
    V2 = cv::Mat(3, 1, CV_64FC1, v2);  //列向量
    V1 = cv::Mat(3, 1, CV_64FC1, v1);  //列向量
    V1 = H * V2;
    corners.left_bottom.x = v1[0] / v1[2];
    corners.left_bottom.y = v1[1] / v1[2];

    //右上角(src.cols,0,1)
    v2[0] = src.cols;
    v2[1] = 0;
    v2[2] = 1;
    V2 = cv::Mat(3, 1, CV_64FC1, v2);  //列向量
    V1 = cv::Mat(3, 1, CV_64FC1, v1);  //列向量
    V1 = H * V2;
    corners.right_top.x = v1[0] / v1[2];
    corners.right_top.y = v1[1] / v1[2];

    //右下角(src.cols,src.rows,1)
    v2[0] = src.cols;
    v2[1] = src.rows;
    v2[2] = 1;
    V2 = cv::Mat(3, 1, CV_64FC1, v2);  //列向量
    V1 = cv::Mat(3, 1, CV_64FC1, v1);  //列向量
    V1 = H * V2;
    corners.right_bottom.x = v1[0] / v1[2];
    corners.right_bottom.y = v1[1] / v1[2];
}

void FrameProcessor::OptimizeSeam(cv::Mat &img1, cv::Mat &trans, cv::Mat &dst) {
    int start = MIN(corners.left_top.x, corners.left_bottom.x);//开始位置，即重叠区域的左边界
    double processWidth = img1.cols - start;//重叠区域的宽度
    int rows = dst.rows;
    int cols = img1.cols;	//注意，是列数*通道数
    double alpha = 1;		//img1中像素的权重
    for (int i = 0; i < rows; i++)
    {
        uchar* p = img1.ptr<uchar>(i);  //获取第i行的首地址
        uchar* t = trans.ptr<uchar>(i);
        uchar* d = dst.ptr<uchar>(i);
        for (int j = start; j < cols; j++)
        {
            //如果遇到图像trans中无像素的黑点，则完全拷贝img1中的数据
            if (t[j * 3] == 0 && t[j * 3 + 1] == 0 && t[j * 3 + 2] == 0)
            {
                alpha = 1;
            }
            else
            {
                //img1中像素的权重，与当前处理点距重叠区域左边界的距离成正比，实验证明，这种方法确实好
                alpha = (processWidth - (j - start)) / processWidth;
            }
            d[j * 3] = p[j * 3] * alpha + t[j * 3] * (1 - alpha);
            d[j * 3 + 1] = p[j * 3 + 1] * alpha + t[j * 3 + 1] * (1 - alpha);
            d[j * 3 + 2] = p[j * 3 + 2] * alpha + t[j * 3 + 2] * (1 - alpha);
        }
    }
}



int FrameProcessor::surfCombine(std::vector<cv::Mat> frameBucket) {
    /**
     * @description: 软拼接
     * 用SIFT算法来实现图像拼接是很常用的方法，但是因为SIFT计算量很大，所以在速度要求很高的场合下不再适用。
     * 所以，它的改进方法SURF因为在速度方面有了明显的提高（速度是SIFT的3倍），所以在图像拼接领域还是大有作为。
     * 虽说SURF精确度和稳定性不及SIFT，但是其综合能力还是优越一些。
     */
    int num_images = frameBucket.size();

    //灰度图转换
    std::vector<cv::Mat> images(num_images);
    for (int i = 0; i < num_images; i++){
        cvtColor(frameBucket[i], images[i], CV_RGB2GRAY);
    }

    cv::Ptr<cv::xfeatures2d::SURF> surf;

    surf = cv::xfeatures2d::SURF::create(800);

    //实例化一个暴力匹配器
    cv::BFMatcher matcher;
    cv::Mat c, d;
    std::vector<cv::KeyPoint> key1, key2;

    //DMatch是用来描述匹配好的一对特征点的类，包含这两个点之间的相关信息
    //比如左图有个特征m，它和右图的特征点n最匹配，这个DMatch就记录它俩最匹配，并且还记录m和n的
    //特征向量的距离和其他信息，这个距离在后面用来做筛选
    std::vector<cv::DMatch> matches;


    //输入图像，输入掩码，输入特征点，输出Mat，存放所有特征点的描述向量
    //这个Mat行数为特征点的个数，列数为每个特征向量的尺寸，SURF是64（维）
    surf->detectAndCompute(frameBucket[0], cv::Mat(), key1, c);
    surf->detectAndCompute(frameBucket[1], cv::Mat(), key2, d);

    std::cout << "descriptor1 depth" << c.depth() << ",type=" << c.type() << std::endl;
    std::cout << "descriptor2 depth" << d.depth() << ",type=" << d.type() << std::endl;

    //匹配，数据来源是特征向量，结果存放在DMatch类型里面
    matcher.match(c, d, matches);

    //sort函数对数据进行升序排列
    //筛选匹配点，根据match里面特征对的距离从小到大排序
    sort(matches.begin(), matches.end());
    std::vector<cv::DMatch> good_matches;
    int ptsPairs = std::min(50, (int)(matches.size() * 0.15));

    for (int i = 0; i < ptsPairs; i++)
    {
        good_matches.push_back(matches[i]);//距离最小的50个压入新的DMatch
    }

    //drawMatches这个函数直接画出摆在一起的图
    cv::Mat outImg;
    //绘制匹配点
    drawMatches(frameBucket[0], key1, frameBucket[1], key2, good_matches, outImg,
                cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    cv::imwrite("test/out/surf_match.png", outImg);

    //计算图像配准点
    std::vector<cv::Point2f> imagePoints1, imagePoints2;

    for (auto & good_match : good_matches)
    {
        imagePoints1.push_back(key1[good_match.queryIdx].pt);
        imagePoints2.push_back(key2[good_match.trainIdx].pt);
    }

    //获取图像1到图像2的投影映射矩阵 尺寸为3*3
    cv::Mat homo = findHomography(imagePoints2, imagePoints1, CV_RANSAC);
    //也可以使用getPerspectiveTransform方法获得透视变换矩阵，不过要求只能有4个点，效果稍差
    //Mat   homo=getPerspectiveTransform(imagePoints1,imagePoints2);
    std::cout << "变换矩阵为：\n" << homo << std::endl; //输出映射矩阵

    //计算配准图的四个顶点坐标
    CalcCorners(homo, frameBucket[1]);
    std::cout << "left_top:" << corners.left_top << std::endl;
    std::cout << "left_bottom:" << corners.left_bottom << std::endl;
    std::cout << "right_top:" << corners.right_top << std::endl;
    std::cout << "right_bottom:" << corners.right_bottom << std::endl;

    //图像配准
    cv::Mat imageTransform1, imageTransform2;
    warpPerspective(frameBucket[1], imageTransform2, homo, cv::Size(MAX(corners.right_top.x, corners.right_bottom.x), frameBucket[0].rows));
    //warpPerspective(a, imageTransform2, adjustMat*homo, Size(b.cols*1.3, b.rows*1.8));
    imwrite("test/out/surf_trans.jpg", imageTransform2);

    //创建拼接后的图,需提前计算图的大小
    int dst_width = imageTransform2.cols;  //取最右点的长度为拼接图的长度
    int dst_height = frameBucket[0].rows;

    cv::Mat dst(dst_height, dst_width, CV_8UC3);
    dst.setTo(0);

    imageTransform2.copyTo(dst(cv::Rect(0, 0, imageTransform2.cols, imageTransform2.rows)));
    frameBucket[0].copyTo(dst(cv::Rect(0, 0, frameBucket[0].cols, frameBucket[0].rows)));

    cv::imwrite("test/out/surf_result.jpg", dst);
    OptimizeSeam(frameBucket[0], imageTransform2, dst);
    cv::imwrite("test/out/opm_surf_result.jpg", dst);

    return 0;
}


cv::Mat FrameProcessor::orbCombine(cv::Mat &src, cv::Mat &transform) {
    /**
     * @description: ORB（Oriented FAST and Rotated BRIEF）是一种快速特征点提取和描述的算法。
     * ORB特征是将FAST特征点的检测方法与BRIEF特征描述子结合起来，并在它们原来的基础上做了改进与优化。据说，ORB算法的速度是sift的100倍，是surf的10倍。
     */


    //提取特征点
    cv::Ptr<cv::ORB> orb = cv::ORB::create(800);

    // 保存特征点
    std::vector<cv::KeyPoint> keyPoint1;
    std::vector<cv::KeyPoint> keyPoint2;

    // 特征描述子
    cv::Mat descriptor1, descriptor2;
    orb->detectAndCompute(src, cv::Mat(), keyPoint1, descriptor1);
    orb->detectAndCompute(transform, cv::Mat(), keyPoint2, descriptor2);


    // Flann匹配, 算法更快但是找到的是最近邻近似匹配，所以当我们需要找到一个相对好的匹配但是不需要最佳匹配的时候往往使用FlannBasedMatcher.
    // BRIEF与ORB特征是二进制的CV_8U而SIFT与SURF特征数据是浮点数，FLANN默认的匹配是基于浮点数运算计算距离，所以导致了类型不支持错误，
    // 这个时候只要使用如下的方法重新构造一下FLANN指针，然后调用match方法即可。
    //    cv::Ptr<cv::DescriptorMatcher> matcher = cv::makePtr<cv::FlannBasedMatcher>(cv::makePtr<cv::flann::LshIndexParams>(12, 20, 2));
    cv::BFMatcher matcher;
    // 保存匹配的结果
    std::vector<cv::DMatch>matches;


    //在descriptor_2中匹配descriptor_1中含有的特征描述子匹配
    matcher.match(descriptor1, descriptor2, matches, cv::Mat());
    sort(matches.begin(), matches.end());
//    std::cout << "matches.size=" << matches.size() << std::endl;

    std::vector<cv::DMatch> good_matches;
    int ptsPairs = std::min(50, (int)(matches.size() * 0.15));
//    std::cout << ptsPairs << std::endl;
    for (int i = 0; i < ptsPairs; i++)
    {
        good_matches.push_back(matches[i]);//距离最小的50个压入新的DMatch
    }

    //    cv::Mat outImg;
    //    //drawMatches这个函数直接画出摆在一起的图
    //    //绘制匹配点
    //    drawMatches(src, keyPoint1, transform, keyPoint2, good_matches, outImg,
    //                cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    //    cv::imwrite("test/out/orb_combine.png", outImg);

    //计算图像配准点
    std::vector<cv::Point2f> imagePoints1, imagePoints2;

    for (auto & good_match : good_matches)
    {
        imagePoints1.push_back(keyPoint1[good_match.queryIdx].pt);
        imagePoints2.push_back(keyPoint2[good_match.trainIdx].pt);
    }

    //获取图像1到图像2的投影映射矩阵 尺寸为3*3

    cv::Mat homo = findHomography(imagePoints2, imagePoints1, CV_RANSAC);
    //也可以使用getPerspectiveTransform方法获得透视变换矩阵，不过要求只能有4个点，效果稍差
    //Mat   homo=getPerspectiveTransform(imagePoints1,imagePoints2);

//    //获取最强配对点在原始图像和矩阵变换后图像上的对应位置，用于图像拼接点的定位
//    cv::Mat adjustMat = (cv::Mat_<double>(3,3) << 1.0,0,src.cols,0,1.0,0,0,0,1.0);
//    cv::Mat adjustHomo = adjustMat * homo;

    //计算配准图的四个顶点坐标
    CalcCorners(homo, transform);

    //图像配准
    cv::Mat imageTransform;
    warpPerspective(transform, imageTransform, homo, cv::Size(MAX(corners.right_top.x, corners.right_bottom.x), src.rows));
    //warpPerspective(a, imageTransform2, adjustMat*homo, Size(b.cols*1.3, b.rows*1.8));

    //    imwrite("test/out/orb_trans.jpg", imageTransform);

    //创建拼接后的图,需提前计算图的大小
    //取最右点的长度为拼接图的长度
    int dst_width = imageTransform.cols;
    int dst_height = src.rows;

    cv::Mat dst(dst_height, dst_width, CV_8UC3);
    dst.setTo(0);

    imageTransform.copyTo(dst(cv::Rect(0, 0, imageTransform.cols, imageTransform.rows)));
    src.copyTo(dst(cv::Rect(0, 0, src.cols, src.rows)));

    //    cv::imwrite("test/out/orb_result.jpg", dst);
    OptimizeSeam(src, imageTransform, dst);

    return dst;
}




