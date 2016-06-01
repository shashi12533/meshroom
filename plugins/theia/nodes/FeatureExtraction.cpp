#include "FeatureExtraction.hpp"
#include <theia/theia.h>
#include <QCommandLineParser>
#include <QDebug>

using namespace std;
using namespace dg;

FeatureExtraction::FeatureExtraction(string nodeName)
    : Node(nodeName)
{
    inputs = {make_ptr<Plug>(Attribute::Type::PATH, "images", *this)};
    output = make_ptr<Plug>(Attribute::Type::PATH, "features", *this);
}

vector<Command> FeatureExtraction::prepare(Cache& cache, bool& blocking)
{
    vector<Command> commands;
    auto p = plug("images");
    for(auto& input : cache.slots(p))
    {
        size_t hash = cache.reserve(*this, {input});
        if(!cache.exists(hash))
        {
            Command c({
                "-m", "compute",                  // mode
                "-t", type(),                     // type
                "-i", cache.location(input->key), // input
                "-o", cache.location(hash)        // output
            });
            commands.emplace_back(c);
        }
    }
    return commands;
}

void FeatureExtraction::compute(const vector<string>& arguments) const
{
    qDebug() << "[FeatureExtraction]";

    // command line options
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addOptions({
        {{"i", "input"}, "image file", "file"},
        {{"o", "output"}, "exif file", "file"},
    });

    // command line parsing
    parser.parse(QCoreApplication::arguments());
    if(!parser.isSet("input") || !parser.isSet("output"))
    {
        qCritical() << "missing command line value";
        return;
    }
    string input = parser.value("input").toStdString();
    string output = parser.value("output").toStdString();
    size_t numthreads = 8;
    size_t maxnumfeatures = 16384;
    size_t numoctaves = -1;
    size_t numlevels = 3;
    size_t firstoctave = 0;
    float edgethreshold = 5.0f;
    float peakthreshold = 0.5f;
    bool rootsift = true;
    bool uprightsift = true;

    // string input;
    // string output;
    //
    // // parse node arguments
    // for(int i = 0; i < arguments.size(); ++i)
    // {
    //     const string& option = arguments[i];
    //     if(matches(option, "--input", "-i"))
    //         getArgs(arguments, ++i, input);
    //     else if(matches(option, "--output", "-o"))
    //         getArgs(arguments, ++i, output);
    //     else if(matches(option, "--threads", "-th"))
    //         getArgs(arguments, ++i, numthreads);
    //     else if(matches(option, "--maxfeatures", "-mf"))
    //         getArgs(arguments, ++i, maxnumfeatures);
    //     else if(matches(option, "--numoctaves", "-no"))
    //         getArgs(arguments, ++i, numoctaves);
    //     else if(matches(option, "--numlevels", "-nl"))
    //         getArgs(arguments, ++i, numlevels);
    //     else if(matches(option, "--firstoctave", "-fo"))
    //         getArgs(arguments, ++i, firstoctave);
    //     else if(matches(option, "--edgethreshold", "-et"))
    //         getArgs(arguments, ++i, edgethreshold);
    //     else if(matches(option, "--peakthreshold", "-pt"))
    //         getArgs(arguments, ++i, peakthreshold);
    //     else if(matches(option, "--rootsift", "-rs"))
    //         getArgs(arguments, ++i, rootsift);
    //     else if(matches(option, "--uprightsift", "-us"))
    //         getArgs(arguments, ++i, uprightsift);
    // }
    //
    // if(input.empty() || output.empty())
    //     throw logic_error("missing command line value");

    // set up the feature extractor
    theia::FeatureExtractor::Options options;
    options.descriptor_extractor_type = theia::DescriptorExtractorType::SIFT;
    options.num_threads = numthreads;
    options.max_num_features = maxnumfeatures;
    options.sift_parameters.num_octaves = numoctaves;
    options.sift_parameters.num_levels = numlevels;
    options.sift_parameters.first_octave = firstoctave;
    options.sift_parameters.edge_threshold = edgethreshold;
    options.sift_parameters.peak_threshold = peakthreshold;
    options.sift_parameters.root_sift = rootsift;
    options.sift_parameters.upright_sift = uprightsift;

    // extract features
    theia::FeatureExtractor extractor(options);
    vector<vector<theia::Keypoint>> keypoints;
    vector<vector<Eigen::VectorXf>> descriptors;
    if(!extractor.Extract({input}, &keypoints, &descriptors))
    {
        qCritical() << " | ERROR (extraction)";
        return;
    }

    // export
    if(!theia::WriteKeypointsAndDescriptors(output, keypoints[0], descriptors[0]))
    {
        qCritical() << " | ERROR (export)";
        return;
    }
}
