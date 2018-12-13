# SCDrone : Self-Control Drone

Projet final dans le cadre de notre cours de Vision dans le programme d'informatique industrielle du Cégep de Lévis-Lauzon.

- [SCDrone : Self-Control Drone](#scdrone--self-control-drone)
  - [Objectif](#objectif)
  - [Technologie utilisé](#technologie-utilis%C3%A9)
  - [Implémentation du SDK du ARDrone 2.0](#impl%C3%A9mentation-du-sdk-du-ardrone-20)
    - [Mes classes de bases](#mes-classes-de-bases)
    - [Thread AT](#thread-at)
    - [Thread NavData](#thread-navdata)
    - [Thread Control](#thread-control)
    - [Thread Video](#thread-video)
    - [Utilisation des la librairie de controler pour crée une application](#utilisation-des-la-librairie-de-controler-pour-cr%C3%A9e-une-application)
  - [Ardrone GUI](#ardrone-gui)
  - [Qu'es-ce qu'on n'a implémenter finalement avec le drone ?](#ques-ce-quon-na-impl%C3%A9menter-finalement-avec-le-drone)
    - [Design a prendre](#design-a-prendre)

## Objectif

Crée une application en c++ avec OpenCV qui controlera automatiquement un drone (AR Drone 2.0) dans un cas et environnement prédéfinis par l'application.

Exemple d'appliction possible :

* Voler dans un parcours avec des boîtes a esquiver et des anneaux a traverser pour attérir sur une cible d'hélicoptère
* Identifier une cible comme une personne ou un signe et le suivre à une distance prédéterminer
* Mapper une environnement et s'y déplacer automatiquement pour détecter des intrus ( comme un agent de sécurité )

Aucune application n'est définit pour l'instant mais pour les accomplirs la même base doit être crée avec le drone pour executer l'application.

Le drone fournit par l'école est un AR Drone 2.0 qui vient avec un SDK fournit par la compagnie Parrot.
Mais pour des raisons de plaisirs et compatibilité nous avons décider de réecrire le SDK en c++ moderne.
Pour ce faire nous avons comme références :

* [SDK Parrot]("http://developer.parrot.com/docs/SDK2") :  lien pour télécharger le code source
* [Documentation SDK Parrot](https://drive.google.com/open?id=13GH4rXcP_LP_JtIrr1bc2artrZ25EUrx) : documentation du drone
* [Port du SDK en C# et C++/cli](https://github.com/ARDrone2Windows/SDK) : le github d'un projet en C# qui a réimplémenter le sdk 

## Technologie utilisé 

Les technologies suivantes sont utilisé par au moins une composante : 

* [C++17](https://en.wikipedia.org/wiki/C%2B%2B11) : nous utilisons c++17 car nous roulons exculisvement sur des compilateurs récent et je voulais utilisé std::filesystem
* [Qt](https://www.qt.io/developers/) : nous allons utilisé boost pour les composantes suivantes :
    * QNetwork   : pour communication TCP et UDP (j'aimerais crisser ca la)
* [OpenCV 3.4.3](https://docs.opencv.org/3.4.3/) : pour le traitement d'image et nous allons utilisé les fonctionnalités cuda et contrib pour le tracking et la détection d'object.
* [FFMPEG](https://www.ffmpeg.org/documentation.html) : utiliser pour faire le decoding du flux video depuis le drone
* [CMake](https://cmake.org/documentation/) : comme système de build pour le projet
* [GLEW](https://github.com/catchorg/Catch2) : Systeme pour charger le context OpenGL
* [GLFW](https://github.com/) : Systeme pour gerer les fenetres et les inputs
* [ASIO](https://github.com/) : Librairie pour le networking
* [STL](www.cppreferences.com) : Utiliser les fonctions de la STL pour les thread et pour gerer le file system.

Ce choix de librairie permet à notre sdk et notre application d'être compilé sur toute les plateformes majeurs
sans problème sauf pour la dépendence à cuda qui coupe la possiblité d'être executer sur rasberry pi, android,
mingw parmis tant d'autre. Mais va nous permettre j'imagine d'avoir un gain de performance pour réduire le délais
de controle du drone.

## Implémentation du SDK du ARDrone 2.0

La première partit à compléter pour pouvoir accomplir le projet est d'implémenter notre SDK qui ressemble grosso-modo à :

* Thread AT : thread qui s'occupe d'envoyer les commandes de contrôle et de configuration au drone
* Thread NavData : thread qui s'occupe de la réception des informations de pilotage du drone
* Thread Control : thread qui s'occupe de la réception des informations de configuration du drone
* Thread Video : thread qui s'occupe de la réception du flux vidéo du drone

Et dans cette ensemble de thread viennent s'ajouter les threads de l'application qui serait :
* Thread Staging : thread qui s'occupe de la conversion du flux vidéo vers le format désirer pour l'application
* Thread App : thread qui recoit les informations de pilotages et vidéos et qui envoie des commandes pour ajuster

![Diagram](ressources/sdk_fonctionnement.png)


### Mes classes de bases


```c++


class Runnable {
protected:
    std::promise<void>	exitSignal;
    std::future<void>	futureObject;

    std::atomic<bool>	state = false;

public:
    virtual ~Runnable() = default;
    Runnable() :futureObject(exitSignal.get_future()) {}
    Runnable(Runnable && obj) : exitSignal(std::move(obj.exitSignal)), futureObject(std::move(obj.futureObject)) {
        std::cout << "Move Constructor called" << std::endl;
    }
    Runnable & operator=(Runnable && obj) {
        std::cout << "Move assignment is called" << std::endl;
        exitSignal = std::move(obj.exitSignal);
        futureObject = std::move(obj.futureObject);
        return *this;
    }

    // les classes enfants doivent donn� une d�finition
    virtual void run_service() = 0;

    // La fonction thread a executer
    std::thread start() {
        return std::thread([this] { this->run_service(); });
    }

    bool stopRequested() const
    {
        return !(futureObject.wait_for(0ms) == std::future_status::timeout);
    }

    virtual void stop() {
        exitSignal.set_value();
    }

    bool isRunning()
    {
        return state;
    }

};

template<typename dataType>
class ConcurrentQueue {
private:
    std::queue<dataType>	queue;
    std::mutex				mutex;

    std::condition_variable cv;

    std::atomic<bool>		forceExit = false;

public:

    dataType& pop()
    {
        std::unique_lock<std::mutex> lk(mutex);
        cv.wait(lk, [this] { return !queue.empty(); }); // Attend que la queue ne soit plus vide et utiliser
        dataType& f = queue.front();
        queue.pop();
        return f;
    }

    dataType pop2() {
        std::unique_lock<std::mutex> lk(mutex);
        cv.wait(lk, [this] { return !queue.empty(); }); // Attend que la queue ne soit plus vide et utiliser
        dataType f = queue.front();
        queue.pop();
        return f;
    }

    dataType pop2_wait(std::chrono::milliseconds timeout, bool* has_data) {
        std::unique_lock<std::mutex> lk(mutex);
        if (!cv.wait_for(lk, timeout, [this] { return !queue.empty(); }))
        {
            *has_data = false;
            return dataType();
        }
        *has_data = true;
        dataType f = queue.front();
        queue.pop();
        return f;
    }

    dataType pop_wait(std::chrono::milliseconds timeout,bool* has_data)
    {
        std::unique_lock<std::mutex> lk(mutex);
        if(!cv.wait_for(lk, timeout, [this] { return !queue.empty(); }))
        {
            *has_data = false;
            return dataType();
        }
        *has_data = true;
        dataType f = queue.front();
        queue.pop();
        return f;
    }

    dataType* pop_all_wait(std::chrono::milliseconds timeout,int* nbrElement)
    {
        std::unique_lock<std::mutex> lk(mutex);
        if (!cv.wait_for(lk, timeout, [this] { return !queue.empty(); }))
        {
            *nbrElement = 0;
            return nullptr;
        }
        *nbrElement = queue.size();
        dataType* d = new dataType[*nbrElement];
        for(int i =0; i<*nbrElement;i++)
        {
            d[i] = queue.front();
            queue.pop();
        }
        return d;

    }

    void push(dataType const& data) {
        forceExit.store(false);
        std::unique_lock<std::mutex> lk(mutex);
        queue.push(data);
        lk.unlock();
        cv.notify_one();
    }

    void empty()
    {
        std::unique_lock<std::mutex> lk(mutex);
        for(int i = 0; i < queue.size(); i++)
        {
            queue.pop();
        }
        lk.unlock();
    }

    bool isEmpty() {
        std::unique_lock<std::mutex> lk(mutex);
        return queue.empty();
    }

};



```


### Thread AT

Thread qui s'occupe de l'envoi des trames de contrôles au drone. Pour que le drone soit controller fluidement
l'interval entre les messages envoier doit être de 30ms.

C'est cette thread qui controle l'etats du drone ( s'il est en vole  , arrêter ou en mode urgence ). Et qui envoie
des les messages de configuration du drone.

Il y a plusieurs facons d'utiliser la thread AT :

* Avec la Queue de message
* En utilisant directement les variables atomiques pour controler ce qui le drone fait quand il ne recoit pas de message
... de la queue
* En utilisant la classe DroneControle qui est un wrapper autour de la Queue avec des taches async pour envoyer constament un message jusqu'a l'arreter de la tache

```c++
class ATClient : public QObject, public Runnable
{
	ATQueue* queue;

	QUdpSocket* socket;
	QHostAddress* sender;
	quint16 port;

	std::atomic<ref_flags>			ref_mode;
	std::atomic<progressive_flags>	prog_flag;
	std::atomic<speed>				speed_drone;

	int sequence_nbr;

public:
	ATClient(ATQueue* queue,QObject* parent = 0);
	~ATClient();
	
	void setVector2D(float x, float y);

    void setSpeedX(x_direction d, float x);
	void setSpeedY(y_direction d, float y);
	void setSpeedZ(z_direction d, float z);
	void setSpeedR(x_direction d, float r);

	void hover();

	void setProgressiveFlag(progressive_flags f)
	{
		prog_flag = f;
	}

	speed getSpeed() const
	{
		return speed_drone;
	}
	progressive_flags getProgressiveFlag() const
	{
		return prog_flag;
	}

	void set_ref(ref_flags f);

	const char* get_ref() const
	{
		ref_flags f = ref_mode;
		switch(f)
		{
		case EMERGENCY_FLAG:
			return "EMERGENCY";
		case TAKEOFF_FLAG:
			return "FLYING";
		default:
			return "LANDED";
		}
		
	}


public slots:
	void run_service();

private slots:
	void on_read_ready();
};

```


### Thread NavData

Thread qui recoit les messages suivants d'information de navigation. Il y a plusieurs informations possibles de recevoir mais nous utilisions le message
de base qui contient toute les informations que nous avons besoin


```c++

#define NAVDATA_HEADER                  0x55667788

#define NAVDATA_MAX_SIZE                4096
#define NAVDATA_MAX_CUSTOM_TIME_SAVE    20


typedef struct _navdata_option_t {
    uint16_t    tag;   // Tag pour l'option spécifique
    uint16_t    size;  // Longeur de la structure

    uint8_t     data[];
} navdata_option_t;


typedef struct _navdata_t {
    uint32_t    header;
    uint32_t    ardrone_state;
    uint32_t    sequence;
    bool_t      vision_defined;

	navdata_option_t* options;
} navdata_t;

/**
 * @brief Minimal navigation data for all flights.
 */
typedef struct _navdata_demo_t {
  uint16_t    tag;					  /*!< Navdata block ('option') identifier */
  uint16_t    size;					  /*!< set this to the size of this structure */

  uint32_t    ctrl_state;             /*!< Flying state (landed, flying, hovering, etc.) defined in CTRL_STATES enum. */
  uint32_t    vbat_flying_percentage; /*!< battery voltage filtered (mV) */

  float   theta;                  /*!< UAV's pitch in milli-degrees */
  float   phi;                    /*!< UAV's roll  in milli-degrees */
  float   psi;                    /*!< UAV's yaw   in milli-degrees */

  int32_t     altitude;               /*!< UAV's altitude in centimeters */

  float   vx;                     /*!< UAV's estimated linear velocity */
  float   vy;                     /*!< UAV's estimated linear velocity */
  float   vz;                     /*!< UAV's estimated linear velocity */

  uint32_t    num_frames;			  /*!< streamed frame index */ // Not used -> To integrate in video stage.

  // Camera parameters compute by detection
  matrix33_t  detection_camera_rot;   /*!<  Deprecated ! Don't use ! */
  vector31_t  detection_camera_trans; /*!<  Deprecated ! Don't use ! */
  uint32_t	  detection_tag_index;    /*!<  Deprecated ! Don't use ! */

  uint32_t	  detection_camera_type;  /*!<  Type of tag searched in detection */

  // Camera parameters compute by drone
  matrix33_t  drone_camera_rot;		  /*!<  Deprecated ! Don't use ! */
  vector31_t  drone_camera_trans;	  /*!<  Deprecated ! Don't use ! */
} navdata_demo_t;

```

Vu la simpliciter de cette thread, le nombre de message que nous recevons est beaucoup trop volumineux pour nos besoin. Donc au lieu d'utiliser
une queue pour echanger les informations, j'utilise une variable atomique qui contient le plus récent message.

Il aurra pu être interessent d'implementer un systeme pour voire la variation des différentes variables sur une temps prédéfinit ou demander sur le fly.
Pour rentabiliser la thread et permettre d'avoir des informations de vol appronfondie.


### Thread Control

Pas implémenter dans l'application pour le moment.

### Thread Video

Le flux vidéo utilisé par le drone est H264 ( MPEG4.10 AVC ) et il peut être configuré avec les options suivantes :

* FPS : entre 15 et 30
* Bitrate : entre 250kbps et 4Mbps
* Résolution : 360p (640x360) ou 720p (1280x720)

Les structures et les définitions du flux vidéo sont définit dans libardrone/include/video_common.h
Et ce qu'il en résulte comme flux vidéo une fois reconstruit est la structure suivante qui représente une frame :

```c++
struct VideoPacket
{
	long Timestamp;
	long Duration;
	unsigned int FrameNumber;
	unsigned short Height;
	unsigned short Width;
	frame_type_t FrameType;
	ARBuffer Buffer;
}
```
Les frames sont de deux type : IDR-Frame ( une frame complète de référence ) et les P-Frame ( frame d'update ).

```c++

class VideoStaging : public Runnable
{
	video_encapsulation_t		prev_encapsulation_ = {}; // Dernier header PaVE re�u

	AVPixelFormat				format_in;					// Format pixel du flux video
	AVPixelFormat				format_out;					// Format pixel de l'image de sortit pour OpenCV
	int							display_width;				// Largeur de l'image dans le stream
	int							display_height;				// Hauteur de l'image dans le stream
	int							bit_rate;					// Bit-rate du stream re�u
	int							fps;						// FPS du stream re�u

#ifdef DEBUG_VIDEO_STAGING
	int								frame_lost = 0;
	TimePoint						start_gap;
	TimePoint						end_gap;
	TimePoint						last_start;
	TimePoint						last_end;
	std::vector<long>				times;
#endif
	AVCodec*					codec;						// Contient les informations du codec qu'on utilise (H264)
	AVCodecContext*				codec_ctx;					// Contient le context de notre codec ( information sur comment decoder le stream )
	AVCodecParserContext*		codec_parser;				// Contient le context sur comment parser les frame de notre stream


	AVFrame*					frame;						// Contient la derni�re frame qu'on na re�u a reconstruire de notre stream
	AVFrame*					frame_output;				// Contient la derni�re frame convertit dans le format pour l'envoyer a OpenCV
	AVPacket*					packet;

	PacketBuffer<H264_INBUF_SIZE> packet_buffer;			// Structure qui contient mon buffer que je remplit pou avoir une frame


	int							line_size;

	SwsContext*					img_convert_ctx;			// Contient le context pour effectuer la conversion entre notre image YUV420 et BGR pour OpenCV

	int							first_frame;				// Contient le num�ro de la premi�re frame re�u
	int							last_frame;					// Contient le num�ro de la derni�re frame re�u

	bool						have_received;				// Indique si on n'a d�j� commencer a recevoir des trames
	bool						only_idr = true;			// Indique si on souhaite selon parser les frames IDR et de skipper les frames P

	atomic<bool>				record_to_file_raw = false; // Indique si on souhaite sauvegarder le stream dans un fichier

	fs::path					record_folder = fs::path("./recording");		// Indique le chemin du fichier a sauvegarder le stream
	int							stream_index = 0;
	std::ofstream				of;

	bool						thread_mode = false;
	VFQueue*					queue;						// La queue dans laquelle les frames sont recu  si on n'est en mode MultiThread Video

	MQueue*						mqueue;                     // Queue pour envoyer les mat.

	std::atomic<video_staging_info> staging_info;
	

public:
	VideoStaging(MQueue* mqueue);
	VideoStaging(VFQueue* queue, MQueue* mqueue);
	//VideoStaging(VFQueue* queue, const char* filepath);
	~VideoStaging();

	int	init() const;

	void run_service() override;

	void set_raw_recording(bool state);

	void onNewVideoFrame(VideoFrame& vf);

	video_staging_info getInfo()
	{
		return staging_info;
	}

private:

	bool have_frame_changed(const VideoFrame& vf);
	bool add_frame_buffer(const VideoFrame& vf);
	void init_or_frame_changed(const VideoFrame& vf,bool init = false);
	bool frame_to_mat(const AVFrame* avframe, cv::Mat& m);
	void append_file(const VideoFrame& vf);
};

```

### Utilisation des la librairie de controler pour crée une application

Voici la classe de base qu'on peut implémenter pour simplifier le démarrage d'une nouvelle
applicaton avec le drone 

```c++
/**
 * \brief DroneClient is the mother class to create a application with the ARDrone
 */
class DroneClient {
protected:
	~DroneClient() = default;

	bool				united_video_thread;

	VFQueue				vf_queue;
	MQueue				mat_queue;
	ATQueue				at_queue;
	NAVQueue			nav_queue;

	VideoStaging		video_staging;	
	VideoClient			video_client;
	ATClient			at_client;
	NavDataClient		nd_client;
	DroneControl		control;
private:
	std::thread			vs_thread;
	std::thread			vc_thread;
	std::thread			at_thread;
	std::thread			nd_thread;

public:
	DroneClient();
	DroneClient(bool lol = true);

	int Start();

	bool isAllThreadRunning();

	virtual void mainLoop() = 0;
protected:

	int init();
	int stop();

};
```


Programe d'exemple

```c++

#include "drone_client.h"


class DroneKB : public DroneClient
{
public:
	virtual ~DroneKB() = default;

	DroneKB() : last_mat(640,360,CV_8UC3,cv::Scalar(0,0,0)), presentation_mat(last_mat.clone())
	{
		nd = {};
		speedXZ = 0.1f;
		speedYR = 0.4f;
	}

private:

	cv::Mat					last_mat;
	cv::Mat					presentation_mat;

	navdata_demo_t			nd;


	void mainLoop() override
	{
		cv::Mat m;
		const char* wname = "Drone video stream";
		cv::namedWindow(wname);

		bool has_image = false;
		bool has_navdata = false;

        // Demande de recevoir les navada_demo
		string navconf = at_format_config("general:navdata_demo", "TRUE");
		navconf.append(at_format_ack());
		at_queue.push(navconf);

		for (;;)
		{
			m = mat_queue.pop2_wait(100ms,&has_image);
			nd = nd_client.get_last_nd_demo();
			if (has_image)
				last_mat = m;

			cv::imshow(wname, last_mat);

		}
		cv::destroyAllWindows();
	}
};


int main(int argc,char* argv[])
{
	DroneKB kb;
	return kb.Start();
}
```

## Ardrone GUI

Pour l'interface visuelle , nous utilisons la librairie ImGui pour nous permmettre de facilement controler les différents parametrage. Pour affichier
le flux vidéos j'utilise simplement des textures OpenGL qui j'update a chaque frame avec les pointeurs de données recus pour chaqu'un des channels.

Les textures ce déplace selon le nombre de crée et supporte jusqu'a 4 videos simultané pour pouvoir acceder à ;

* Video du drone
* Video de traitement lors de tracking
* Camera local pour faire du tracking de cette caméra
* ....



## Qu'es-ce qu'on n'a implémenter finalement avec le drone ?

La première étapde dans nos objectifs était d'être capable de faire un tracking simple avec des thresholds pour suivre un objets sur deux axes (x,y).


### Design a prendre

Du la nature multithreader du code et qu'il s'agit d'une application qui doit intéragir en temps réal et que nous devons toujours pourvoir puller les
derniers informations du drone pour bien agir en conséquence.

