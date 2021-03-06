#include "alrecorder.h"

AlRecorder::AlRecorder(QObject *parent) : QObject(parent) {
  this->outputFilePath = QDir::currentPath() + QDir::separator() + "ouy.webm";
}

void AlRecorder::startSlot() { this->start(); }

void AlRecorder::stopSlot() {
  qDebug() << "stop fired";
  if (this->m_pipeline) { // pipeline exists - destroy it
    // send an end-of-stream event to flush metadata and cause an EosMessage to
    // be delivered
    this->m_pipeline->sendEvent(QGst::EosEvent::create());
  } else {
    this->stop();
  }
}

QGst::BinPtr AlRecorder::createAudioSrcBin() {
  QGst::BinPtr audioBin;

  try {
    //        audioBin = QGst::Bin::fromDescription("autoaudiosrc
    //        name=\"audiosrc\" ! audioconvert ! "
    //                                              "audioresample ! audiorate !
    //                                              vorbisenc ! queue");

    audioBin = QGst::Bin::fromDescription(
        "autoaudiosrc name=\"audiosrc\" ! audioconvert ! "
        "audioresample ! audiorate ! lamemp3enc ! queue");

  } catch (const QGlib::Error &error) {
    qCritical() << "Failed to create audio source bin:" << error;
    return QGst::BinPtr();
  }

  QGst::ElementPtr src = audioBin->getElementByName("audiosrc");
  // autoaudiosrc creates the actual source in the READY state
  src->setState(QGst::StateReady);
  return audioBin;
}

QGst::BinPtr AlRecorder::createVideoSrcBin() {
  QGst::BinPtr videoBin;
  QString rawvideocaps =
      QString("video/x-raw,format=RGB,width=1280,height=480,framerate=25/"
              "1,pixel-aspect-ratio=1/1");

  //    QString outputPipeDesc = QString(" appsrc name=videosrc caps=\"%1\"
  //    is-live=true format=time do-timestamp=true ! videorate !"
  //                                     " videoconvert ! vp8enc threads=2
  //                                     deadline=1 cpu-used=15
  //                                     target-bitrate=32768000 ! queue")

  QString outputPipeDesc =
      QString(
          " appsrc name=videosrc caps=\"%1\" is-live=true format=time "
          "do-timestamp=true ! videorate !"
          " videoconvert ! x264enc quantizer=0 pass=5 speed-preset=1 ! queue")
          .arg(rawvideocaps);

  try {
    videoBin = QGst::Bin::fromDescription(outputPipeDesc);
    this->m_src.setElement(videoBin->getElementByName("videosrc"));
    return videoBin;
  } catch (const QGlib::Error &error) {
    qCritical() << "Failed to create video source bin:" << error;
    return QGst::BinPtr();
  }
  return videoBin;
}

void AlRecorder::start() {
  QDateTime now = QDateTime::currentDateTime();

  qDebug() << now.toString("yyyy_MM_dd-hh:mm:ss");
  this->outputFilePath = QDir::currentPath() + QDir::separator() + "out" +
                         QDir::separator() +
                         now.toString("yyyy_MM_dd-hh:mm:ss") + ".mp4";

  QGst::BinPtr audioSrcBin = createAudioSrcBin();
  QGst::BinPtr videoSrcBin = createVideoSrcBin();
  //    QGst::ElementPtr mux = QGst::ElementFactory::make("webmmux");
  QGst::ElementPtr mux = QGst::ElementFactory::make("matroskamux");
  QGst::ElementPtr sink = QGst::ElementFactory::make("filesink");

  if (!audioSrcBin || !videoSrcBin || !mux || !sink) {
    qDebug() << tr("One or more elements could not be created. Verify that you "
                   "have all the necessary element plugins installed.");
    return;
  }

  sink->setProperty("location", this->outputFilePath);

  m_pipeline = QGst::Pipeline::create();
  m_pipeline->add(audioSrcBin, videoSrcBin, mux, sink);

  // link elements
  QGst::PadPtr audioPad = mux->getRequestPad("audio_%u");

  audioSrcBin->getStaticPad("src")->link(audioPad);

  QGst::PadPtr videoPad = mux->getRequestPad("video_%u");
  videoSrcBin->getStaticPad("src")->link(videoPad);

  mux->link(sink);

  // connect the bus
  m_pipeline->bus()->addSignalWatch();
  QGlib::connect(m_pipeline->bus(), "message", this, &AlRecorder::onBusMessage);

  // go!
  m_pipeline->setState(QGst::StatePlaying);
  this->statusText = tr("Stop recording");
}

void AlRecorder::stop() {
  // stop recording
  m_pipeline->setState(QGst::StateNull);

  // clear the pointer, destroying the pipeline as its reference count drops to
  // zero.
  m_pipeline.clear();

  this->statusText = tr("Start recording");
}

void AlRecorder::onBusMessage(const QGst::MessagePtr &message) {
  switch (message->type()) {
  case QGst::MessageEos:
    // got end-of-stream - stop the pipeline
    //        qDebug() << "got EOS";
    stop();
    break;
  case QGst::MessageError:
    // check if the pipeline exists before destroying it,
    // as we might get multiple error messages
    if (m_pipeline) {

      stop();
    }
    qDebug() << tr("Pipeline Error") << " | "
             << message.staticCast<QGst::ErrorMessage>()->error().message();
    break;
  default:
    break;
  }
}
