/**
 * QtAwesome - use font-awesome (or other font icons) in your c++ / Qt Application
 *
 * MIT Licensed
 *
 * Copyright 2013-2016 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "QtAwesome.h"
#include "QtAwesomeAnim.h"

#include <QDebug>
#include <QFile>
#include <QFontDatabase>



/// The font-awesome icon painter
class QtAwesomeCharIconPainter: public QtAwesomeIconPainter
{

protected:

    QStringList optionKeysForModeAndState( const QString& key, QIcon::Mode mode, QIcon::State state)
    {
        QString modePostfix;
        switch(mode) {
            case QIcon::Disabled:
                modePostfix = "-disabled";
                break;
            case QIcon::Active:
                modePostfix = "-active";
                break;
            case QIcon::Selected:
                modePostfix = "-selected";
                break;
            default: // QIcon::Normal:
                // modePostfix = "";
                break;
        }

        QString statePostfix;
        if( state == QIcon::Off) {
            statePostfix = "-off";
        }

        // the keys that need to bet tested:   key-mode-state | key-mode | key-state | key
        QStringList result;
        if( !modePostfix.isEmpty() ) {
            if( !statePostfix.isEmpty() ) {
                result.push_back( key + modePostfix + statePostfix );
            }
            result.push_back( key + modePostfix );
        }
        if( !statePostfix.isEmpty() ) {
            result.push_back( key + statePostfix );
        }
        return result;
    }


    QVariant optionValueForModeAndState( const QString& baseKey, QIcon::Mode mode, QIcon::State state, const QVariantMap& options )
    {
        foreach( QString key, optionKeysForModeAndState(baseKey, mode, state) ) {
            //if ( options.contains(key) && options.value(key).toString().isEmpty()) qDebug() << "Not found:" << key;
            if( options.contains(key) && !(options.value(key).toString().isEmpty()) )
                return options.value(key);
        }

        return options.value(baseKey);
    }

public:


    virtual void paint( QtAwesome* awesome, QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state, const QVariantMap& options  )
    {
        painter->save();

        QVariant var =options.value("anim");
        QtAwesomeAnimation* anim = var.value<QtAwesomeAnimation*>();
        if( anim ) {
            anim->setup( *painter, rect );
        }

        // set the default options
        QColor color = optionValueForModeAndState("color", mode, state, options).value<QColor>();
        QString text = optionValueForModeAndState("text", mode, state, options).toString();

        Q_ASSERT(color.isValid());
        Q_ASSERT(!text.isEmpty());

        painter->setPen(color);

        // add some 'padding' around the icon
        int drawSize = qRound(rect.height()*options.value("scale-factor").toFloat());

        painter->setFont( awesome->font(drawSize) );
        painter->drawText( rect, text, QTextOption( Qt::AlignCenter|Qt::AlignVCenter ) );
        painter->restore();
    }

};


//---------------------------------------------------------------------------------------


/// The painter icon engine.
class QtAwesomeIconPainterIconEngine : public QIconEngine
{

public:

    QtAwesomeIconPainterIconEngine( QtAwesome* awesome, QtAwesomeIconPainter* painter, const QVariantMap& options  )
        : awesomeRef_(awesome)
        , iconPainterRef_(painter)
        , options_(options)
    {
    }

    virtual ~QtAwesomeIconPainterIconEngine() {}

    QtAwesomeIconPainterIconEngine* clone() const
    {
        return new QtAwesomeIconPainterIconEngine( awesomeRef_, iconPainterRef_, options_ );
    }

    virtual void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
    {
        Q_UNUSED( mode );
        Q_UNUSED( state );
        iconPainterRef_->paint( awesomeRef_, painter, rect, mode, state, options_ );
    }

    virtual QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
    {
        QPixmap pm(size);
        pm.fill( Qt::transparent ); // we need transparency
        {
            QPainter p(&pm);
            paint(&p, QRect(QPoint(0,0),size), mode, state);
        }
        return pm;
    }

private:

    QtAwesome* awesomeRef_;                  ///< a reference to the QtAwesome instance
    QtAwesomeIconPainter* iconPainterRef_;   ///< a reference to the icon painter
    QVariantMap options_;                    ///< the options for this icon painter
};


//---------------------------------------------------------------------------------------

/// The default icon colors
QtAwesome::QtAwesome( QObject* parent )
    : QObject( parent )
    , namedCodepoints_()
{
    // initialize the default options
    setDefaultOption( "color", QColor(50,50,50) );
    setDefaultOption( "color-disabled", QColor(70,70,70,60));
    setDefaultOption( "color-active", QColor(10,10,10));
    setDefaultOption( "color-selected", QColor(10,10,10));
    setDefaultOption( "scale-factor", 0.9 );

    setDefaultOption( "text", QVariant() );
    setDefaultOption( "text-disabled", QVariant() );
    setDefaultOption( "text-active", QVariant() );
    setDefaultOption( "text-selected", QVariant() );

    fontIconPainter_ = new QtAwesomeCharIconPainter();

}


QtAwesome::~QtAwesome()
{
    delete fontIconPainter_;
//    delete errorIconPainter_;
    qDeleteAll(painterMap_);
}

/// initializes the QtAwesome icon factory with the given fontname
void QtAwesome::init(const QString& fontname)
{
    fontName_ = fontname;
}

struct FANameIcon {
  const char *name;
  fa::icon icon;
};

static const FANameIcon faNameIconArray[] = {
      { "fa_500px"                         , fa::fa_500px                         },
      { "addressbook"                      , fa::addressbook                      },
      { "addressbooko"                     , fa::addressbooko                     },
      { "addresscard"                      , fa::addresscard                      },
      { "addresscardo"                     , fa::addresscardo                     },
      { "adjust"                           , fa::adjust                           },
      { "adn"                              , fa::adn                              },
      { "aligncenter"                      , fa::aligncenter                      },
      { "alignjustify"                     , fa::alignjustify                     },
      { "alignleft"                        , fa::alignleft                        },
      { "alignright"                       , fa::alignright                       },
      { "amazon"                           , fa::amazon                           },
      { "ambulance"                        , fa::ambulance                        },
      { "americansignlanguageinterpreting" , fa::americansignlanguageinterpreting },
      { "anchor"                           , fa::anchor                           },
      { "android"                          , fa::android                          },
      { "angellist"                        , fa::angellist                        },
      { "angledoubledown"                  , fa::angledoubledown                  },
      { "angledoubleleft"                  , fa::angledoubleleft                  },
      { "angledoubleright"                 , fa::angledoubleright                 },
      { "angledoubleup"                    , fa::angledoubleup                    },
      { "angledown"                        , fa::angledown                        },
      { "angleleft"                        , fa::angleleft                        },
      { "angleright"                       , fa::angleright                       },
      { "angleup"                          , fa::angleup                          },
      { "apple"                            , fa::apple                            },
      { "archive"                          , fa::archive                          },
      { "areachart"                        , fa::areachart                        },
      { "arrowcircledown"                  , fa::arrowcircledown                  },
      { "arrowcircleleft"                  , fa::arrowcircleleft                  },
      { "arrowcircleodown"                 , fa::arrowcircleodown                 },
      { "arrowcircleoleft"                 , fa::arrowcircleoleft                 },
      { "arrowcircleoright"                , fa::arrowcircleoright                },
      { "arrowcircleoup"                   , fa::arrowcircleoup                   },
      { "arrowcircleright"                 , fa::arrowcircleright                 },
      { "arrowcircleup"                    , fa::arrowcircleup                    },
      { "arrowdown"                        , fa::arrowdown                        },
      { "arrowleft"                        , fa::arrowleft                        },
      { "arrowright"                       , fa::arrowright                       },
      { "arrowup"                          , fa::arrowup                          },
      { "arrows"                           , fa::arrows                           },
      { "arrowsalt"                        , fa::arrowsalt                        },
      { "arrowsh"                          , fa::arrowsh                          },
      { "arrowsv"                          , fa::arrowsv                          },
      { "aslinterpreting"                  , fa::aslinterpreting                  },
      { "assistivelisteningsystems"        , fa::assistivelisteningsystems        },
      { "asterisk"                         , fa::asterisk                         },
      { "at"                               , fa::at                               },
      { "audiodescription"                 , fa::audiodescription                 },
      { "automobile"                       , fa::automobile                       },
      { "backward"                         , fa::backward                         },
      { "balancescale"                     , fa::balancescale                     },
      { "ban"                              , fa::ban                              },
      { "bandcamp"                         , fa::bandcamp                         },
      { "bank"                             , fa::bank                             },
      { "barchart"                         , fa::barchart                         },
      { "barcharto"                        , fa::barcharto                        },
      { "barcode"                          , fa::barcode                          },
      { "bars"                             , fa::bars                             },
      { "bath"                             , fa::bath                             },
      { "bathtub"                          , fa::bathtub                          },
      { "battery"                          , fa::battery                          },
      { "battery0"                         , fa::battery0                         },
      { "battery1"                         , fa::battery1                         },
      { "battery2"                         , fa::battery2                         },
      { "battery3"                         , fa::battery3                         },
      { "battery4"                         , fa::battery4                         },
      { "batteryempty"                     , fa::batteryempty                     },
      { "batteryfull"                      , fa::batteryfull                      },
      { "batteryhalf"                      , fa::batteryhalf                      },
      { "batteryquarter"                   , fa::batteryquarter                   },
      { "batterythreequarters"             , fa::batterythreequarters             },
      { "bed"                              , fa::bed                              },
      { "beer"                             , fa::beer                             },
      { "behance"                          , fa::behance                          },
      { "behancesquare"                    , fa::behancesquare                    },
      { "bell"                             , fa::bell                             },
      { "bello"                            , fa::bello                            },
      { "bellslash"                        , fa::bellslash                        },
      { "bellslasho"                       , fa::bellslasho                       },
      { "bicycle"                          , fa::bicycle                          },
      { "binoculars"                       , fa::binoculars                       },
      { "birthdaycake"                     , fa::birthdaycake                     },
      { "bitbucket"                        , fa::bitbucket                        },
      { "bitbucketsquare"                  , fa::bitbucketsquare                  },
      { "bitcoin"                          , fa::bitcoin                          },
      { "blacktie"                         , fa::blacktie                         },
      { "blind"                            , fa::blind                            },
      { "bluetooth"                        , fa::bluetooth                        },
      { "bluetoothb"                       , fa::bluetoothb                       },
      { "bold"                             , fa::bold                             },
      { "bolt"                             , fa::bolt                             },
      { "bomb"                             , fa::bomb                             },
      { "book"                             , fa::book                             },
      { "bookmark"                         , fa::bookmark                         },
      { "bookmarko"                        , fa::bookmarko                        },
      { "braille"                          , fa::braille                          },
      { "briefcase"                        , fa::briefcase                        },
      { "btc"                              , fa::btc                              },
      { "bug"                              , fa::bug                              },
      { "building"                         , fa::building                         },
      { "buildingo"                        , fa::buildingo                        },
      { "bullhorn"                         , fa::bullhorn                         },
      { "bullseye"                         , fa::bullseye                         },
      { "bus"                              , fa::bus                              },
      { "buysellads"                       , fa::buysellads                       },
      { "cab"                              , fa::cab                              },
      { "calculator"                       , fa::calculator                       },
      { "calendar"                         , fa::calendar                         },
      { "calendarchecko"                   , fa::calendarchecko                   },
      { "calendarminuso"                   , fa::calendarminuso                   },
      { "calendaro"                        , fa::calendaro                        },
      { "calendarpluso"                    , fa::calendarpluso                    },
      { "calendartimeso"                   , fa::calendartimeso                   },
      { "camera"                           , fa::camera                           },
      { "cameraretro"                      , fa::cameraretro                      },
      { "car"                              , fa::car                              },
      { "caretdown"                        , fa::caretdown                        },
      { "caretleft"                        , fa::caretleft                        },
      { "caretright"                       , fa::caretright                       },
      { "caretsquareodown"                 , fa::caretsquareodown                 },
      { "caretsquareoleft"                 , fa::caretsquareoleft                 },
      { "caretsquareoright"                , fa::caretsquareoright                },
      { "caretsquareoup"                   , fa::caretsquareoup                   },
      { "caretup"                          , fa::caretup                          },
      { "cartarrowdown"                    , fa::cartarrowdown                    },
      { "cartplus"                         , fa::cartplus                         },
      { "cc"                               , fa::cc                               },
      { "ccamex"                           , fa::ccamex                           },
      { "ccdinersclub"                     , fa::ccdinersclub                     },
      { "ccdiscover"                       , fa::ccdiscover                       },
      { "ccjcb"                            , fa::ccjcb                            },
      { "ccmastercard"                     , fa::ccmastercard                     },
      { "ccpaypal"                         , fa::ccpaypal                         },
      { "ccstripe"                         , fa::ccstripe                         },
      { "ccvisa"                           , fa::ccvisa                           },
      { "certificate"                      , fa::certificate                      },
      { "chain"                            , fa::chain                            },
      { "chainbroken"                      , fa::chainbroken                      },
      { "check"                            , fa::check                            },
      { "checkcircle"                      , fa::checkcircle                      },
      { "checkcircleo"                     , fa::checkcircleo                     },
      { "checksquare"                      , fa::checksquare                      },
      { "checksquareo"                     , fa::checksquareo                     },
      { "chevroncircledown"                , fa::chevroncircledown                },
      { "chevroncircleleft"                , fa::chevroncircleleft                },
      { "chevroncircleright"               , fa::chevroncircleright               },
      { "chevroncircleup"                  , fa::chevroncircleup                  },
      { "chevrondown"                      , fa::chevrondown                      },
      { "chevronleft"                      , fa::chevronleft                      },
      { "chevronright"                     , fa::chevronright                     },
      { "chevronup"                        , fa::chevronup                        },
      { "child"                            , fa::child                            },
      { "chrome"                           , fa::chrome                           },
      { "circle"                           , fa::circle                           },
      { "circleo"                          , fa::circleo                          },
      { "circleonotch"                     , fa::circleonotch                     },
      { "circlethin"                       , fa::circlethin                       },
      { "clipboard"                        , fa::clipboard                        },
      { "clocko"                           , fa::clocko                           },
      { "clone"                            , fa::clone                            },
      { "close"                            , fa::close                            },
      { "cloud"                            , fa::cloud                            },
      { "clouddownload"                    , fa::clouddownload                    },
      { "cloudupload"                      , fa::cloudupload                      },
      { "cny"                              , fa::cny                              },
      { "code"                             , fa::code                             },
      { "codefork"                         , fa::codefork                         },
      { "codepen"                          , fa::codepen                          },
      { "codiepie"                         , fa::codiepie                         },
      { "coffee"                           , fa::coffee                           },
      { "cog"                              , fa::cog                              },
      { "cogs"                             , fa::cogs                             },
      { "columns"                          , fa::columns                          },
      { "comment"                          , fa::comment                          },
      { "commento"                         , fa::commento                         },
      { "commenting"                       , fa::commenting                       },
      { "commentingo"                      , fa::commentingo                      },
      { "comments"                         , fa::comments                         },
      { "commentso"                        , fa::commentso                        },
      { "compass"                          , fa::compass                          },
      { "compress"                         , fa::compress                         },
      { "connectdevelop"                   , fa::connectdevelop                   },
      { "contao"                           , fa::contao                           },
      { "copy"                             , fa::copy                             },
      { "copyright"                        , fa::copyright                        },
      { "creativecommons"                  , fa::creativecommons                  },
      { "creditcard"                       , fa::creditcard                       },
      { "creditcardalt"                    , fa::creditcardalt                    },
      { "crop"                             , fa::crop                             },
      { "crosshairs"                       , fa::crosshairs                       },
      { "css3"                             , fa::css3                             },
      { "cube"                             , fa::cube                             },
      { "cubes"                            , fa::cubes                            },
      { "cut"                              , fa::cut                              },
      { "cutlery"                          , fa::cutlery                          },
      { "dashboard"                        , fa::dashboard                        },
      { "dashcube"                         , fa::dashcube                         },
      { "database"                         , fa::database                         },
      { "deaf"                             , fa::deaf                             },
      { "deafness"                         , fa::deafness                         },
      { "dedent"                           , fa::dedent                           },
      { "delicious"                        , fa::delicious                        },
      { "desktop"                          , fa::desktop                          },
      { "deviantart"                       , fa::deviantart                       },
      { "diamond"                          , fa::diamond                          },
      { "digg"                             , fa::digg                             },
      { "dollar"                           , fa::dollar                           },
      { "dotcircleo"                       , fa::dotcircleo                       },
      { "download"                         , fa::download                         },
      { "dribbble"                         , fa::dribbble                         },
      { "driverslicense"                   , fa::driverslicense                   },
      { "driverslicenseo"                  , fa::driverslicenseo                  },
      { "dropbox"                          , fa::dropbox                          },
      { "drupal"                           , fa::drupal                           },
      { "edge"                             , fa::edge                             },
      { "edit"                             , fa::edit                             },
      { "eercast"                          , fa::eercast                          },
      { "eject"                            , fa::eject                            },
      { "ellipsish"                        , fa::ellipsish                        },
      { "ellipsisv"                        , fa::ellipsisv                        },
      { "empire"                           , fa::empire                           },
      { "envelope"                         , fa::envelope                         },
      { "envelopeo"                        , fa::envelopeo                        },
      { "envelopeopen"                     , fa::envelopeopen                     },
      { "envelopeopeno"                    , fa::envelopeopeno                    },
      { "envelopesquare"                   , fa::envelopesquare                   },
      { "envira"                           , fa::envira                           },
      { "eraser"                           , fa::eraser                           },
      { "etsy"                             , fa::etsy                             },
      { "eur"                              , fa::eur                              },
      { "euro"                             , fa::euro                             },
      { "exchange"                         , fa::exchange                         },
      { "exclamation"                      , fa::exclamation                      },
      { "exclamationcircle"                , fa::exclamationcircle                },
      { "exclamationtriangle"              , fa::exclamationtriangle              },
      { "expand"                           , fa::expand                           },
      { "expeditedssl"                     , fa::expeditedssl                     },
      { "externallink"                     , fa::externallink                     },
      { "externallinksquare"               , fa::externallinksquare               },
      { "eye"                              , fa::eye                              },
      { "eyeslash"                         , fa::eyeslash                         },
      { "eyedropper"                       , fa::eyedropper                       },
      { "fa"                               , fa::fa                               },
      { "facebook"                         , fa::facebook                         },
      { "facebookf"                        , fa::facebookf                        },
      { "facebookofficial"                 , fa::facebookofficial                 },
      { "facebooksquare"                   , fa::facebooksquare                   },
      { "fastbackward"                     , fa::fastbackward                     },
      { "fastforward"                      , fa::fastforward                      },
      { "fax"                              , fa::fax                              },
      { "feed"                             , fa::feed                             },
      { "female"                           , fa::female                           },
      { "fighterjet"                       , fa::fighterjet                       },
      { "file"                             , fa::file                             },
      { "filearchiveo"                     , fa::filearchiveo                     },
      { "fileaudioo"                       , fa::fileaudioo                       },
      { "filecodeo"                        , fa::filecodeo                        },
      { "fileexcelo"                       , fa::fileexcelo                       },
      { "fileimageo"                       , fa::fileimageo                       },
      { "filemovieo"                       , fa::filemovieo                       },
      { "fileo"                            , fa::fileo                            },
      { "filepdfo"                         , fa::filepdfo                         },
      { "filephotoo"                       , fa::filephotoo                       },
      { "filepictureo"                     , fa::filepictureo                     },
      { "filepowerpointo"                  , fa::filepowerpointo                  },
      { "filesoundo"                       , fa::filesoundo                       },
      { "filetext"                         , fa::filetext                         },
      { "filetexto"                        , fa::filetexto                        },
      { "filevideoo"                       , fa::filevideoo                       },
      { "filewordo"                        , fa::filewordo                        },
      { "filezipo"                         , fa::filezipo                         },
      { "fileso"                           , fa::fileso                           },
      { "film"                             , fa::film                             },
      { "filter"                           , fa::filter                           },
      { "fire"                             , fa::fire                             },
      { "fireextinguisher"                 , fa::fireextinguisher                 },
      { "firefox"                          , fa::firefox                          },
      { "firstorder"                       , fa::firstorder                       },
      { "flag"                             , fa::flag                             },
      { "flagcheckered"                    , fa::flagcheckered                    },
      { "flago"                            , fa::flago                            },
      { "flash"                            , fa::flash                            },
      { "flask"                            , fa::flask                            },
      { "flickr"                           , fa::flickr                           },
      { "floppyo"                          , fa::floppyo                          },
      { "folder"                           , fa::folder                           },
      { "foldero"                          , fa::foldero                          },
      { "folderopen"                       , fa::folderopen                       },
      { "folderopeno"                      , fa::folderopeno                      },
      { "font"                             , fa::font                             },
      { "fontawesome"                      , fa::fontawesome                      },
      { "fonticons"                        , fa::fonticons                        },
      { "fortawesome"                      , fa::fortawesome                      },
      { "forumbee"                         , fa::forumbee                         },
      { "forward"                          , fa::forward                          },
      { "foursquare"                       , fa::foursquare                       },
      { "freecodecamp"                     , fa::freecodecamp                     },
      { "frowno"                           , fa::frowno                           },
      { "futbolo"                          , fa::futbolo                          },
      { "gamepad"                          , fa::gamepad                          },
      { "gavel"                            , fa::gavel                            },
      { "gbp"                              , fa::gbp                              },
      { "ge"                               , fa::ge                               },
      { "gear"                             , fa::gear                             },
      { "gears"                            , fa::gears                            },
      { "genderless"                       , fa::genderless                       },
      { "getpocket"                        , fa::getpocket                        },
      { "gg"                               , fa::gg                               },
      { "ggcircle"                         , fa::ggcircle                         },
      { "gift"                             , fa::gift                             },
      { "git"                              , fa::git                              },
      { "gitsquare"                        , fa::gitsquare                        },
      { "github"                           , fa::github                           },
      { "githubalt"                        , fa::githubalt                        },
      { "githubsquare"                     , fa::githubsquare                     },
      { "gitlab"                           , fa::gitlab                           },
      { "gittip"                           , fa::gittip                           },
      { "glass"                            , fa::glass                            },
      { "glide"                            , fa::glide                            },
      { "glideg"                           , fa::glideg                           },
      { "globe"                            , fa::globe                            },
      { "google"                           , fa::google                           },
      { "googleplus"                       , fa::googleplus                       },
      { "googlepluscircle"                 , fa::googlepluscircle                 },
      { "googleplusofficial"               , fa::googleplusofficial               },
      { "googleplussquare"                 , fa::googleplussquare                 },
      { "googlewallet"                     , fa::googlewallet                     },
      { "graduationcap"                    , fa::graduationcap                    },
      { "gratipay"                         , fa::gratipay                         },
      { "grav"                             , fa::grav                             },
      { "group"                            , fa::group                            },
      { "hsquare"                          , fa::hsquare                          },
      { "hackernews"                       , fa::hackernews                       },
      { "handgrabo"                        , fa::handgrabo                        },
      { "handlizardo"                      , fa::handlizardo                      },
      { "handodown"                        , fa::handodown                        },
      { "handoleft"                        , fa::handoleft                        },
      { "handoright"                       , fa::handoright                       },
      { "handoup"                          , fa::handoup                          },
      { "handpapero"                       , fa::handpapero                       },
      { "handpeaceo"                       , fa::handpeaceo                       },
      { "handpointero"                     , fa::handpointero                     },
      { "handrocko"                        , fa::handrocko                        },
      { "handscissorso"                    , fa::handscissorso                    },
      { "handspocko"                       , fa::handspocko                       },
      { "handstopo"                        , fa::handstopo                        },
      { "handshakeo"                       , fa::handshakeo                       },
      { "hardofhearing"                    , fa::hardofhearing                    },
      { "hashtag"                          , fa::hashtag                          },
      { "hddo"                             , fa::hddo                             },
      { "header"                           , fa::header                           },
      { "headphones"                       , fa::headphones                       },
      { "heart"                            , fa::heart                            },
      { "hearto"                           , fa::hearto                           },
      { "heartbeat"                        , fa::heartbeat                        },
      { "history"                          , fa::history                          },
      { "home"                             , fa::home                             },
      { "hospitalo"                        , fa::hospitalo                        },
      { "hotel"                            , fa::hotel                            },
      { "hourglass"                        , fa::hourglass                        },
      { "hourglass1"                       , fa::hourglass1                       },
      { "hourglass2"                       , fa::hourglass2                       },
      { "hourglass3"                       , fa::hourglass3                       },
      { "hourglassend"                     , fa::hourglassend                     },
      { "hourglasshalf"                    , fa::hourglasshalf                    },
      { "hourglasso"                       , fa::hourglasso                       },
      { "hourglassstart"                   , fa::hourglassstart                   },
      { "houzz"                            , fa::houzz                            },
      { "html5"                            , fa::html5                            },
      { "icursor"                          , fa::icursor                          },
      { "idbadge"                          , fa::idbadge                          },
      { "idcard"                           , fa::idcard                           },
      { "idcardo"                          , fa::idcardo                          },
      { "ils"                              , fa::ils                              },
      { "image"                            , fa::image                            },
      { "imdb"                             , fa::imdb                             },
      { "inbox"                            , fa::inbox                            },
      { "indent"                           , fa::indent                           },
      { "industry"                         , fa::industry                         },
      { "info"                             , fa::info                             },
      { "infocircle"                       , fa::infocircle                       },
      { "inr"                              , fa::inr                              },
      { "instagram"                        , fa::instagram                        },
      { "institution"                      , fa::institution                      },
      { "internetexplorer"                 , fa::internetexplorer                 },
      { "intersex"                         , fa::intersex                         },
      { "ioxhost"                          , fa::ioxhost                          },
      { "italic"                           , fa::italic                           },
      { "joomla"                           , fa::joomla                           },
      { "jpy"                              , fa::jpy                              },
      { "jsfiddle"                         , fa::jsfiddle                         },
      { "key"                              , fa::key                              },
      { "keyboardo"                        , fa::keyboardo                        },
      { "krw"                              , fa::krw                              },
      { "language"                         , fa::language                         },
      { "laptop"                           , fa::laptop                           },
      { "lastfm"                           , fa::lastfm                           },
      { "lastfmsquare"                     , fa::lastfmsquare                     },
      { "leaf"                             , fa::leaf                             },
      { "leanpub"                          , fa::leanpub                          },
      { "legal"                            , fa::legal                            },
      { "lemono"                           , fa::lemono                           },
      { "leveldown"                        , fa::leveldown                        },
      { "levelup"                          , fa::levelup                          },
      { "lifebouy"                         , fa::lifebouy                         },
      { "lifebuoy"                         , fa::lifebuoy                         },
      { "lifering"                         , fa::lifering                         },
      { "lifesaver"                        , fa::lifesaver                        },
      { "lightbulbo"                       , fa::lightbulbo                       },
      { "linechart"                        , fa::linechart                        },
      { "link"                             , fa::link                             },
      { "linkedin"                         , fa::linkedin                         },
      { "linkedinsquare"                   , fa::linkedinsquare                   },
      { "linode"                           , fa::linode                           },
      { "fa_linux"                         , fa::fa_linux                         },
      { "list"                             , fa::list                             },
      { "listalt"                          , fa::listalt                          },
      { "listol"                           , fa::listol                           },
      { "listul"                           , fa::listul                           },
      { "locationarrow"                    , fa::locationarrow                    },
      { "lock"                             , fa::lock                             },
      { "longarrowdown"                    , fa::longarrowdown                    },
      { "longarrowleft"                    , fa::longarrowleft                    },
      { "longarrowright"                   , fa::longarrowright                   },
      { "longarrowup"                      , fa::longarrowup                      },
      { "lowvision"                        , fa::lowvision                        },
      { "magic"                            , fa::magic                            },
      { "magnet"                           , fa::magnet                           },
      { "mailforward"                      , fa::mailforward                      },
      { "mailreply"                        , fa::mailreply                        },
      { "mailreplyall"                     , fa::mailreplyall                     },
      { "male"                             , fa::male                             },
      { "map"                              , fa::map                              },
      { "mapmarker"                        , fa::mapmarker                        },
      { "mapo"                             , fa::mapo                             },
      { "mappin"                           , fa::mappin                           },
      { "mapsigns"                         , fa::mapsigns                         },
      { "mars"                             , fa::mars                             },
      { "marsdouble"                       , fa::marsdouble                       },
      { "marsstroke"                       , fa::marsstroke                       },
      { "marsstrokeh"                      , fa::marsstrokeh                      },
      { "marsstrokev"                      , fa::marsstrokev                      },
      { "maxcdn"                           , fa::maxcdn                           },
      { "meanpath"                         , fa::meanpath                         },
      { "medium"                           , fa::medium                           },
      { "medkit"                           , fa::medkit                           },
      { "meetup"                           , fa::meetup                           },
      { "meho"                             , fa::meho                             },
      { "mercury"                          , fa::mercury                          },
      { "microchip"                        , fa::microchip                        },
      { "microphone"                       , fa::microphone                       },
      { "microphoneslash"                  , fa::microphoneslash                  },
      { "minus"                            , fa::minus                            },
      { "minuscircle"                      , fa::minuscircle                      },
      { "minussquare"                      , fa::minussquare                      },
      { "minussquareo"                     , fa::minussquareo                     },
      { "mixcloud"                         , fa::mixcloud                         },
      { "mobile"                           , fa::mobile                           },
      { "mobilephone"                      , fa::mobilephone                      },
      { "modx"                             , fa::modx                             },
      { "money"                            , fa::money                            },
      { "moono"                            , fa::moono                            },
      { "mortarboard"                      , fa::mortarboard                      },
      { "motorcycle"                       , fa::motorcycle                       },
      { "mousepointer"                     , fa::mousepointer                     },
      { "music"                            , fa::music                            },
      { "navicon"                          , fa::navicon                          },
      { "neuter"                           , fa::neuter                           },
      { "newspapero"                       , fa::newspapero                       },
      { "objectgroup"                      , fa::objectgroup                      },
      { "objectungroup"                    , fa::objectungroup                    },
      { "odnoklassniki"                    , fa::odnoklassniki                    },
      { "odnoklassnikisquare"              , fa::odnoklassnikisquare              },
      { "opencart"                         , fa::opencart                         },
      { "openid"                           , fa::openid                           },
      { "opera"                            , fa::opera                            },
      { "optinmonster"                     , fa::optinmonster                     },
      { "outdent"                          , fa::outdent                          },
      { "pagelines"                        , fa::pagelines                        },
      { "paintbrush"                       , fa::paintbrush                       },
      { "paperplane"                       , fa::paperplane                       },
      { "paperplaneo"                      , fa::paperplaneo                      },
      { "paperclip"                        , fa::paperclip                        },
      { "paragraph"                        , fa::paragraph                        },
      { "paste"                            , fa::paste                            },
      { "pause"                            , fa::pause                            },
      { "pausecircle"                      , fa::pausecircle                      },
      { "pausecircleo"                     , fa::pausecircleo                     },
      { "paw"                              , fa::paw                              },
      { "paypal"                           , fa::paypal                           },
      { "pencil"                           , fa::pencil                           },
      { "pencilsquare"                     , fa::pencilsquare                     },
      { "pencilsquareo"                    , fa::pencilsquareo                    },
      { "percent"                          , fa::percent                          },
      { "phone"                            , fa::phone                            },
      { "phonesquare"                      , fa::phonesquare                      },
      { "photo"                            , fa::photo                            },
      { "pictureo"                         , fa::pictureo                         },
      { "piechart"                         , fa::piechart                         },
      { "piedpiper"                        , fa::piedpiper                        },
      { "piedpiperalt"                     , fa::piedpiperalt                     },
      { "piedpiperpp"                      , fa::piedpiperpp                      },
      { "pinterest"                        , fa::pinterest                        },
      { "pinterestp"                       , fa::pinterestp                       },
      { "pinterestsquare"                  , fa::pinterestsquare                  },
      { "plane"                            , fa::plane                            },
      { "play"                             , fa::play                             },
      { "playcircle"                       , fa::playcircle                       },
      { "playcircleo"                      , fa::playcircleo                      },
      { "plug"                             , fa::plug                             },
      { "plus"                             , fa::plus                             },
      { "pluscircle"                       , fa::pluscircle                       },
      { "plussquare"                       , fa::plussquare                       },
      { "plussquareo"                      , fa::plussquareo                      },
      { "podcast"                          , fa::podcast                          },
      { "poweroff"                         , fa::poweroff                         },
      { "print"                            , fa::print                            },
      { "producthunt"                      , fa::producthunt                      },
      { "puzzlepiece"                      , fa::puzzlepiece                      },
      { "qq"                               , fa::qq                               },
      { "qrcode"                           , fa::qrcode                           },
      { "question"                         , fa::question                         },
      { "questioncircle"                   , fa::questioncircle                   },
      { "questioncircleo"                  , fa::questioncircleo                  },
      { "quora"                            , fa::quora                            },
      { "quoteleft"                        , fa::quoteleft                        },
      { "quoteright"                       , fa::quoteright                       },
      { "ra"                               , fa::ra                               },
      { "random"                           , fa::random                           },
      { "ravelry"                          , fa::ravelry                          },
      { "rebel"                            , fa::rebel                            },
      { "recycle"                          , fa::recycle                          },
      { "reddit"                           , fa::reddit                           },
      { "redditalien"                      , fa::redditalien                      },
      { "redditsquare"                     , fa::redditsquare                     },
      { "refresh"                          , fa::refresh                          },
      { "registered"                       , fa::registered                       },
      { "remove"                           , fa::remove                           },
      { "renren"                           , fa::renren                           },
      { "reorder"                          , fa::reorder                          },
      { "repeat"                           , fa::repeat                           },
      { "reply"                            , fa::reply                            },
      { "replyall"                         , fa::replyall                         },
      { "resistance"                       , fa::resistance                       },
      { "retweet"                          , fa::retweet                          },
      { "rmb"                              , fa::rmb                              },
      { "road"                             , fa::road                             },
      { "rocket"                           , fa::rocket                           },
      { "rotateleft"                       , fa::rotateleft                       },
      { "rotateright"                      , fa::rotateright                      },
      { "rouble"                           , fa::rouble                           },
      { "rss"                              , fa::rss                              },
      { "rsssquare"                        , fa::rsssquare                        },
      { "rub"                              , fa::rub                              },
      { "ruble"                            , fa::ruble                            },
      { "rupee"                            , fa::rupee                            },
      { "s15"                              , fa::s15                              },
      { "safari"                           , fa::safari                           },
      { "save"                             , fa::save                             },
      { "scissors"                         , fa::scissors                         },
      { "scribd"                           , fa::scribd                           },
      { "search"                           , fa::search                           },
      { "searchminus"                      , fa::searchminus                      },
      { "searchplus"                       , fa::searchplus                       },
      { "sellsy"                           , fa::sellsy                           },
      { "send"                             , fa::send                             },
      { "sendo"                            , fa::sendo                            },
      { "server"                           , fa::server                           },
      { "share"                            , fa::share                            },
      { "sharealt"                         , fa::sharealt                         },
      { "sharealtsquare"                   , fa::sharealtsquare                   },
      { "sharesquare"                      , fa::sharesquare                      },
      { "sharesquareo"                     , fa::sharesquareo                     },
      { "shekel"                           , fa::shekel                           },
      { "sheqel"                           , fa::sheqel                           },
      { "shield"                           , fa::shield                           },
      { "ship"                             , fa::ship                             },
      { "shirtsinbulk"                     , fa::shirtsinbulk                     },
      { "shoppingbag"                      , fa::shoppingbag                      },
      { "shoppingbasket"                   , fa::shoppingbasket                   },
      { "shoppingcart"                     , fa::shoppingcart                     },
      { "shower"                           , fa::shower                           },
      { "signin"                           , fa::signin                           },
      { "signlanguage"                     , fa::signlanguage                     },
      { "signout"                          , fa::signout                          },
      { "signal"                           , fa::signal                           },
      { "signing"                          , fa::signing                          },
      { "simplybuilt"                      , fa::simplybuilt                      },
      { "sitemap"                          , fa::sitemap                          },
      { "skyatlas"                         , fa::skyatlas                         },
      { "skype"                            , fa::skype                            },
      { "slack"                            , fa::slack                            },
      { "sliders"                          , fa::sliders                          },
      { "slideshare"                       , fa::slideshare                       },
      { "smileo"                           , fa::smileo                           },
      { "snapchat"                         , fa::snapchat                         },
      { "snapchatghost"                    , fa::snapchatghost                    },
      { "snapchatsquare"                   , fa::snapchatsquare                   },
      { "snowflakeo"                       , fa::snowflakeo                       },
      { "soccerballo"                      , fa::soccerballo                      },
      { "sort"                             , fa::sort                             },
      { "sortalphaasc"                     , fa::sortalphaasc                     },
      { "sortalphadesc"                    , fa::sortalphadesc                    },
      { "sortamountasc"                    , fa::sortamountasc                    },
      { "sortamountdesc"                   , fa::sortamountdesc                   },
      { "sortasc"                          , fa::sortasc                          },
      { "sortdesc"                         , fa::sortdesc                         },
      { "sortdown"                         , fa::sortdown                         },
      { "sortnumericasc"                   , fa::sortnumericasc                   },
      { "sortnumericdesc"                  , fa::sortnumericdesc                  },
      { "sortup"                           , fa::sortup                           },
      { "soundcloud"                       , fa::soundcloud                       },
      { "spaceshuttle"                     , fa::spaceshuttle                     },
      { "spinner"                          , fa::spinner                          },
      { "spoon"                            , fa::spoon                            },
      { "spotify"                          , fa::spotify                          },
      { "square"                           , fa::square                           },
      { "squareo"                          , fa::squareo                          },
      { "stackexchange"                    , fa::stackexchange                    },
      { "stackoverflow"                    , fa::stackoverflow                    },
      { "star"                             , fa::star                             },
      { "starhalf"                         , fa::starhalf                         },
      { "starhalfempty"                    , fa::starhalfempty                    },
      { "starhalffull"                     , fa::starhalffull                     },
      { "starhalfo"                        , fa::starhalfo                        },
      { "staro"                            , fa::staro                            },
      { "steam"                            , fa::steam                            },
      { "steamsquare"                      , fa::steamsquare                      },
      { "stepbackward"                     , fa::stepbackward                     },
      { "stepforward"                      , fa::stepforward                      },
      { "stethoscope"                      , fa::stethoscope                      },
      { "stickynote"                       , fa::stickynote                       },
      { "stickynoteo"                      , fa::stickynoteo                      },
      { "stop"                             , fa::stop                             },
      { "stopcircle"                       , fa::stopcircle                       },
      { "stopcircleo"                      , fa::stopcircleo                      },
      { "streetview"                       , fa::streetview                       },
      { "strikethrough"                    , fa::strikethrough                    },
      { "stumbleupon"                      , fa::stumbleupon                      },
      { "stumbleuponcircle"                , fa::stumbleuponcircle                },
      { "subscript"                        , fa::subscript                        },
      { "subway"                           , fa::subway                           },
      { "suitcase"                         , fa::suitcase                         },
      { "suno"                             , fa::suno                             },
      { "superpowers"                      , fa::superpowers                      },
      { "superscript"                      , fa::superscript                      },
      { "support"                          , fa::support                          },
      { "table"                            , fa::table                            },
      { "tablet"                           , fa::tablet                           },
      { "tachometer"                       , fa::tachometer                       },
      { "tag"                              , fa::tag                              },
      { "tags"                             , fa::tags                             },
      { "tasks"                            , fa::tasks                            },
      { "taxi"                             , fa::taxi                             },
      { "telegram"                         , fa::telegram                         },
      { "television"                       , fa::television                       },
      { "tencentweibo"                     , fa::tencentweibo                     },
      { "terminal"                         , fa::terminal                         },
      { "textheight"                       , fa::textheight                       },
      { "textwidth"                        , fa::textwidth                        },
      { "th"                               , fa::th                               },
      { "thlarge"                          , fa::thlarge                          },
      { "thlist"                           , fa::thlist                           },
      { "themeisle"                        , fa::themeisle                        },
      { "thermometer"                      , fa::thermometer                      },
      { "thermometer0"                     , fa::thermometer0                     },
      { "thermometer1"                     , fa::thermometer1                     },
      { "thermometer2"                     , fa::thermometer2                     },
      { "thermometer3"                     , fa::thermometer3                     },
      { "thermometer4"                     , fa::thermometer4                     },
      { "thermometerempty"                 , fa::thermometerempty                 },
      { "thermometerfull"                  , fa::thermometerfull                  },
      { "thermometerhalf"                  , fa::thermometerhalf                  },
      { "thermometerquarter"               , fa::thermometerquarter               },
      { "thermometerthreequarters"         , fa::thermometerthreequarters         },
      { "thumbtack"                        , fa::thumbtack                        },
      { "thumbsdown"                       , fa::thumbsdown                       },
      { "thumbsodown"                      , fa::thumbsodown                      },
      { "thumbsoup"                        , fa::thumbsoup                        },
      { "thumbsup"                         , fa::thumbsup                         },
      { "ticket"                           , fa::ticket                           },
      { "times"                            , fa::times                            },
      { "timescircle"                      , fa::timescircle                      },
      { "timescircleo"                     , fa::timescircleo                     },
      { "timesrectangle"                   , fa::timesrectangle                   },
      { "timesrectangleo"                  , fa::timesrectangleo                  },
      { "tint"                             , fa::tint                             },
      { "toggledown"                       , fa::toggledown                       },
      { "toggleleft"                       , fa::toggleleft                       },
      { "toggleoff"                        , fa::toggleoff                        },
      { "toggleon"                         , fa::toggleon                         },
      { "toggleright"                      , fa::toggleright                      },
      { "toggleup"                         , fa::toggleup                         },
      { "trademark"                        , fa::trademark                        },
      { "train"                            , fa::train                            },
      { "transgender"                      , fa::transgender                      },
      { "transgenderalt"                   , fa::transgenderalt                   },
      { "trash"                            , fa::trash                            },
      { "trasho"                           , fa::trasho                           },
      { "tree"                             , fa::tree                             },
      { "trello"                           , fa::trello                           },
      { "tripadvisor"                      , fa::tripadvisor                      },
      { "trophy"                           , fa::trophy                           },
      { "truck"                            , fa::truck                            },
      { "fa_try"                           , fa::fa_try                           },
      { "tty"                              , fa::tty                              },
      { "tumblr"                           , fa::tumblr                           },
      { "tumblrsquare"                     , fa::tumblrsquare                     },
      { "turkishlira"                      , fa::turkishlira                      },
      { "tv"                               , fa::tv                               },
      { "twitch"                           , fa::twitch                           },
      { "twitter"                          , fa::twitter                          },
      { "twittersquare"                    , fa::twittersquare                    },
      { "umbrella"                         , fa::umbrella                         },
      { "underline"                        , fa::underline                        },
      { "undo"                             , fa::undo                             },
      { "universalaccess"                  , fa::universalaccess                  },
      { "university"                       , fa::university                       },
      { "unlink"                           , fa::unlink                           },
      { "unlock"                           , fa::unlock                           },
      { "unlockalt"                        , fa::unlockalt                        },
      { "unsorted"                         , fa::unsorted                         },
      { "upload"                           , fa::upload                           },
      { "usb"                              , fa::usb                              },
      { "usd"                              , fa::usd                              },
      { "user"                             , fa::user                             },
      { "usercircle"                       , fa::usercircle                       },
      { "usercircleo"                      , fa::usercircleo                      },
      { "usermd"                           , fa::usermd                           },
      { "usero"                            , fa::usero                            },
      { "userplus"                         , fa::userplus                         },
      { "usersecret"                       , fa::usersecret                       },
      { "usertimes"                        , fa::usertimes                        },
      { "users"                            , fa::users                            },
      { "vcard"                            , fa::vcard                            },
      { "vcardo"                           , fa::vcardo                           },
      { "venus"                            , fa::venus                            },
      { "venusdouble"                      , fa::venusdouble                      },
      { "venusmars"                        , fa::venusmars                        },
      { "viacoin"                          , fa::viacoin                          },
      { "viadeo"                           , fa::viadeo                           },
      { "viadeosquare"                     , fa::viadeosquare                     },
      { "videocamera"                      , fa::videocamera                      },
      { "vimeo"                            , fa::vimeo                            },
      { "vimeosquare"                      , fa::vimeosquare                      },
      { "vine"                             , fa::vine                             },
      { "vk"                               , fa::vk                               },
      { "volumecontrolphone"               , fa::volumecontrolphone               },
      { "volumedown"                       , fa::volumedown                       },
      { "volumeoff"                        , fa::volumeoff                        },
      { "volumeup"                         , fa::volumeup                         },
      { "warning"                          , fa::warning                          },
      { "wechat"                           , fa::wechat                           },
      { "weibo"                            , fa::weibo                            },
      { "weixin"                           , fa::weixin                           },
      { "whatsapp"                         , fa::whatsapp                         },
      { "wheelchair"                       , fa::wheelchair                       },
      { "wheelchairalt"                    , fa::wheelchairalt                    },
      { "wifi"                             , fa::wifi                             },
      { "wikipediaw"                       , fa::wikipediaw                       },
      { "windowclose"                      , fa::windowclose                      },
      { "windowcloseo"                     , fa::windowcloseo                     },
      { "windowmaximize"                   , fa::windowmaximize                   },
      { "windowminimize"                   , fa::windowminimize                   },
      { "windowrestore"                    , fa::windowrestore                    },
      { "windows"                          , fa::windows                          },
      { "won"                              , fa::won                              },
      { "wordpress"                        , fa::wordpress                        },
      { "wpbeginner"                       , fa::wpbeginner                       },
      { "wpexplorer"                       , fa::wpexplorer                       },
      { "wpforms"                          , fa::wpforms                          },
      { "wrench"                           , fa::wrench                           },
      { "xing"                             , fa::xing                             },
      { "xingsquare"                       , fa::xingsquare                       },
      { "ycombinator"                      , fa::ycombinator                      },
      { "ycombinatorsquare"                , fa::ycombinatorsquare                },
      { "yahoo"                            , fa::yahoo                            },
      { "yc"                               , fa::yc                               },
      { "ycsquare"                         , fa::ycsquare                         },
      { "yelp"                             , fa::yelp                             },
      { "yen"                              , fa::yen                              },
      { "yoast"                            , fa::yoast                            },
      { "youtube"                          , fa::youtube                          },
      { "youtubeplay"                      , fa::youtubeplay                      },
      { "youtubesquare"                    , fa::youtubesquare                    }
};


/// a specialized init function so font-awesome is loaded and initialized
/// this method return true on success, it will return false if the fnot cannot be initialized
/// To initialize QtAwesome with font-awesome you need to call this method
bool QtAwesome::initFontAwesome( )
{
    static int fontAwesomeFontId = -1;

    // only load font-awesome once
    if( fontAwesomeFontId < 0 ) {

        // The macro below internally calls "qInitResources_QtAwesome()". this initializes
        // the resource system. For a .pri project this isn't required, but when building and using a
        // static library the resource need to initialized first.
        ///
        // I've checked th qInitResource_* code and calling this method mutliple times shouldn't be any problem
        // (More info about this subject:  http://qt-project.org/wiki/QtResources)
        Q_INIT_RESOURCE(QtAwesome);

        // load the font file
        QFile res(":/fonts/fontawesome-4.7.0.ttf");
        if(!res.open(QIODevice::ReadOnly)) {
            qDebug() << "Font awesome font could not be loaded!";
            return false;
        }
        QByteArray fontData( res.readAll() );
        res.close();

        // fetch the given font
        fontAwesomeFontId = QFontDatabase::addApplicationFontFromData(fontData);
    }

    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(fontAwesomeFontId);
    if( !loadedFontFamilies.empty() ) {
        fontName_= loadedFontFamilies.at(0);
    } else {
        qDebug() << "Font awesome font is empty?!";
        fontAwesomeFontId = -1; // restore the font-awesome id
        return false;
    }

    // intialize the map
    QHash<QString, int>& m = namedCodepoints_;
    for (unsigned i = 0; i < sizeof(faNameIconArray)/sizeof(FANameIcon); ++i) {
      m.insert(faNameIconArray[i].name, faNameIconArray[i].icon);
    }

    return true;
}

void QtAwesome::addNamedCodepoint( const QString& name, int codePoint)
{
    namedCodepoints_.insert( name, codePoint);
}


/// Sets a default option. These options are passed on to the icon painters
void QtAwesome::setDefaultOption(const QString& name, const QVariant& value)
{
    defaultOptions_.insert( name, value );
}


/// Returns the default option for the given name
QVariant QtAwesome::defaultOption(const QString& name)
{
    return defaultOptions_.value( name );
}


// internal helper method to merge to option amps
static QVariantMap mergeOptions( const QVariantMap& defaults, const QVariantMap& override )
{
    QVariantMap result= defaults;
    if( !override.isEmpty() ) {
        QMapIterator<QString,QVariant> itr(override);
        while( itr.hasNext() ) {
            itr.next();
            result.insert( itr.key(), itr.value() );
        }
    }
    return result;
}


/// Creates an icon with the given code-point
/// <code>
///     awesome->icon( icon_group )
/// </code>
QIcon QtAwesome::icon(int character, const QVariantMap &options)
{
    // create a merged QVariantMap to have default options and icon-specific options
    QVariantMap optionMap = mergeOptions( defaultOptions_, options );
    optionMap.insert("text", QString( QChar(static_cast<int>(character)) ) );

    return icon( fontIconPainter_, optionMap );
}



/// Creates an icon with the given name
///
/// You can use the icon names as defined on http://fortawesome.github.io/Font-Awesome/design.html  withour the 'icon-' prefix
/// @param name the name of the icon
/// @param options extra option to pass to the icon renderer
QIcon QtAwesome::icon(const QString& name, const QVariantMap& options)
{
    // when it's a named codepoint
    if( namedCodepoints_.count(name) ) {
        return icon( namedCodepoints_.value(name), options );
    }


    // create a merged QVariantMap to have default options and icon-specific options
    QVariantMap optionMap = mergeOptions( defaultOptions_, options );

    // this method first tries to retrieve the icon
    QtAwesomeIconPainter* painter = painterMap_.value(name);
    if( !painter ) {
        return QIcon();
    }

    return icon( painter, optionMap );
}

/// Create a dynamic icon by simlpy supplying a painter object
/// The ownership of the painter is NOT transfered.
/// @param painter a dynamic painter that is going to paint the icon
/// @param optionmap the options to pass to the painter
QIcon QtAwesome::icon(QtAwesomeIconPainter* painter, const QVariantMap& optionMap )
{
    // Warning, when you use memoryleak detection. You should turn it of for the next call
    // QIcon's placed in gui items are often cached and not deleted when my memory-leak detection checks for leaks.
    // I'm not sure if it's a Qt bug or something I do wrong
    QtAwesomeIconPainterIconEngine* engine = new QtAwesomeIconPainterIconEngine( this, painter, optionMap  );
    return QIcon( engine );
}

/// Adds a named icon-painter to the QtAwesome icon map
/// As the name applies the ownership is passed over to QtAwesome
///
/// @param name the name of the icon
/// @param painter the icon painter to add for this name
void QtAwesome::give(const QString& name, QtAwesomeIconPainter* painter)
{
    delete painterMap_.value( name );   // delete the old one
    painterMap_.insert( name, painter );
}

/// Creates/Gets the icon font with a given size in pixels. This can be usefull to use a label for displaying icons
/// Example:
///
///    QLabel* label = new QLabel( QChar( icon_group ) );
///    label->setFont( awesome->font(16) )
QFont QtAwesome::font( int size )
{
    QFont font( fontName_);
    font.setPixelSize(size);
    return font;
}
