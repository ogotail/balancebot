#!/usr/bin/env python
# -*- coding: utf-8 -*-#

# ================================================================
# ===                            UTP                           ===
# ================================================================

from kivy.support import install_twisted_reactor
install_twisted_reactor()

from twisted.internet.protocol import DatagramProtocol
from twisted.internet import reactor

class Bot(DatagramProtocol):
        
    def __init__( self, host = "192.168.4.1", port = 2390 ):
        self.host = host
        self.port = port
    
    def startProtocol( self ):
        self.transport.connect( self.host, self.port )
        APP.printlog( "now we can only send to host {host} port {port}".format( host=self.host, port=self.port ))
        #APP.send_to_bot("RSVP bonjour") # no need for address
    
    def datagramReceived( self, data, host ):
        #APP.printlog( "received {0} from {1}:{2}".format( data[:-1], host[0], host[1] ))
        APP.on_bot(data)
        
    def sendData( self, text ):
        APP.printlog( "send : " + text )
        try:
            self.transport.write( text + "                " )
        except:
            print "??? WIFI ???"
    
    def connectionRefused(self):
        APP.printlog( "No one listening" )
    
# ================================================================
# ===                          import                          ===
# ================================================================

import kivy
from kivy.app import App
from kivy.lang import Builder

from kivy.garden.navigationdrawer import NavigationDrawer

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.tabbedpanel import TabbedPanelItem
from kivy.uix.togglebutton import ToggleButton
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.uix.actionbar import ActionBar,ActionButton, ActionView, ActionItem
from kivy.uix.scrollview import ScrollView
from kivy.uix.checkbox import CheckBox
from kivy.uix.behaviors import ToggleButtonBehavior
    
from kivy.properties import ObjectProperty, StringProperty, OptionProperty, NumericProperty, BooleanProperty, AliasProperty
from kivy.graphics import Color, Rectangle, Ellipse, Line

# ================================================================
# ===                          Sender                          ===
# ================================================================

class Sender(object):
    msg_sended = StringProperty( "" )

    def receive( self, msg ):
        print "-"+msg+"-"+self.msg_sended+"-"
        if msg == self.msg_sended :
            self.clock.cancel()
            self.disabled = False
        
    def send( self, key, value = "" , disabled = True ):
        self.msg_sended = (str(key) + " " + str(value)).strip()
        self.disabled = disabled 
        self.send_to_bot()
            
    def send_to_bot( self, *args ):
        if APP.CONNECTED:
            APP.send_to_bot( self, self.msg_sended )
            self.clock = Clock.schedule_once( self.send_to_bot, 0.5)
        else:
            APP.printlog( "Not Connected !" )
            self.disabled = False
            
class Receiver(object):
    msg_received = StringProperty( "" )
    key = StringProperty( "" )
    
    #def on_msg_received(self, *args):
        #pass
    
    def on_key( self, *args ):
        print args
        APP.add_receiver( self )
        
# ================================================================
# ===                          Battery                         ===
# ================================================================

Builder.load_string("""
<BatButton>: 
    batval: 0
    icon: 'atlas://myatlas/Bat' + str(root.batval)
    key: 'bat'
""")

class BatButton( ActionButton, Receiver ):
    
    def __init__(self, **kwargs):
        super(BatButton, self).__init__(**kwargs)
        
        #APP.add_receiver( self, 'bat' )
    
    def on_msg_received( self, *args ):
        batval = float( self.msg_received )
        if 4 < batval < 5 :
            self.batval = 10
        else: 
            batval = max( min( int(( batval-6.6 ) // 0.3 ) , 5), 0)
            if self.batval != batval :
                self.batval = batval
    
# ================================================================
# ===                           Pause                          ===
# ================================================================

Builder.load_string('''
<PauseButton>:
    connected: app.CONNECTED
    pause: app.PAUSE
    text: "Not connected"
    couleur: (1,0,0,0.7)
    
    on_connected: self.toggle_state()
    on_pause: self.toggle_state()
    on_press: self.toggle_pause()
    
    canvas.before:
        Color:
            rgba: root.couleur
        Rectangle:
            pos: self.pos
            size: self.size
''')

class PauseButton( ActionButton, Sender ):
    """Met en pause les moteurs"""
    def toggle_state( self ):
        if self.connected :
            if self.pause :
                self.text = '    Pause    '
                self.couleur = (1,0.5,0,0.7)
                self.send( "PAUSE", 1)
            else:
                self.text = '      On     '
                self.couleur = (0,1,0,0.7)
                self.send( "PAUSE", 0)
        else : 
            self.text = 'Not connected'
            self.couleur = (1,0,0,0.7)
            APP.Pause = True
        
    def toggle_pause(self):
        if self.connected:
            APP.PAUSE = not self.pause

# ================================================================
# ===                       Reset encoder                      ===
# ================================================================

Builder.load_string('''
<ResetButton>:
    text: "Reset Encoder"
    on_press: self.send( "ENCODER", 0)
    
    canvas.before:
        Color:
            rgba: (1,0,0,1)
        Rectangle:
            pos: self.pos
            size: self.size
''')

class ResetButton( ActionButton, Sender ):
    pass

# ================================================================
# ===                      Main Interface                      ===
# ================================================================
    
Builder.load_string('''
<TabDrawer>:

    ActionBar
    
        pos_hint: {'top':1}
        
        ActionView:
            use_separator: True
            
            ActionPrevious:
                title: root.current_text
                with_previous: True
                on_press: root.toggle_state()
                
            ResetButton:
            
            PauseButton:
            
            ActionOverflow:
            
            BatButton:
            
            ActionButton:
                icon: 'atlas://data/images/defaulttheme/overflow'
                on_press: root.toggle_menu()
''')

class TabDrawerItem( TabbedPanelItem ):
    menu = BooleanProperty(None)

class TabDrawer( BoxLayout, Sender ):
    current_tab = ObjectProperty(None)
    current_text = StringProperty('BalanceBot')
    main_panel = ObjectProperty(None)
    orientation = 'vertical'
    
    Data = (('Stability',
                (   ('Vertical', ( 0, -5, 5, 0.1)),
                    ('kPs', ( 5, 0, 100 , 0.1 )),
                    ('kIs', ( 5, 0, 100 , 0.1 )),
                    ('kDs', ( 5, 0, 100 , 0.1 )),
                 ),
                (   ('Angle', ( 1, 0, 0 )),
                    ('PID_stab', ( 0, 1, 0 )),
                    ('P_stab', ( 0.5, 0.5, 0 )),
                    ('I_stab', ( 0, 0.5 ,0.5 )),
                    ('D_stab', ( 0, 0, 1 )),
                 )
            ),  
            ('Deplacement',
                (   ('Origine', ( 0, -100, 100, 1 )),
                    ('kPd', ( 5, 0, 10 , 0.1 )),
                    ('kId', ( 5, 0, 10 , 0.1 )),
                    ('kDd', ( 5, 0, 10 , 0.1 )),
                 ),
                (   ('Encoder', ( 1, 0, 0 )),
                    ('PID_dep', ( 0, 1, 0 )),
                    ('P_dep', ( 0.5, 0.5, 0 )),
                    ('I_dep', ( 0, 0.5, 0.5 )),
                    ('D_dep', ( 0, 0, 1 )),
                 )
            ),
            ('Rotation',
                (   ('Direction', ( 0, -100, 100, 1 )),
                    ('kPr', ( 5, 0, 10 , 0.1 )),
                    ('kIr', ( 5, 0, 10 , 0.1 )),
                    ('kDr', ( 5, 0, 10 , 0.1 )),
                 ),
                (   ('Dif_Enc', ( 1, 0, 0 )),
                    ('PID_rot', ( 0, 1, 0 )),
                    ('P_rot', ( 0.5, 0.5, 0 )),
                    ('I_rot', ( 0, 0.5, 0.5 )),
                    ('D_rot', ( 0, 0, 1 )),
                 )
            ))
    
    def __init__(self, **kwargs):
        
        super(TabDrawer, self).__init__(**kwargs)
        
        self.navdraw = NavigationDrawer()
        self.side_panel = BoxLayout( orientation = 'vertical' )
        self.side_panel.tabbed_panel = self
        self.navdraw.add_widget(self.side_panel)
        self.add_widget(self.navdraw)
        
        self.add_widget( ConnectionLayout() )
        
        pad_Layout = TabDrawerItem( text = 'control' )
        pad_Layout.add_widget(Pad())
        self.add_widget( pad_Layout )
        
        for item in self.Data:
            self.add_widget( PidLayout(item) )
            
        self.add_widget( LogLayout() )
        
        
    def add_widget(self, widget, index=0):
        if issubclass(widget.__class__, TabDrawerItem):
        #if len( self.children ) > 2 :
            self.side_panel.add_widget(widget)
            if len( self.side_panel.children ) == 1 :
                self.switch_to(widget)
        else:
            super(TabDrawer, self).add_widget(widget)
            #print widget, len(self.children), len(self.children) > 2
        
    def switch_to(self, header):
        if self.navdraw.state == 'open':
            self.navdraw.toggle_state()
        
        if self.current_tab:
            self.current_tab.state = 'normal'
        header.state = 'down'
        self.current_tab = header
        self.current_text = header.text
        if self.main_panel :
            self.navdraw.remove_widget(self.main_panel)
        self.navdraw.add_widget( header.content ) 
        self.main_panel = header.content
        
    def toggle_state(self):
        self.navdraw.toggle_state()
        
    def on_current_text(self, *args):
        self.send( "get", self.current_text )
        
    def toggle_menu(self):
        self.current_tab.menu = not self.current_tab.menu
            
# ================================================================
# ===                         Connection                       ===
# ================================================================

Builder.load_string("""
<ConnectionLayout>:
    text: 'connection'
    
    BoxLayout:
        id: bl
        orientation: 'vertical'
        spacing: 10
        padding: 10
            
        TextInput:
            id: ti_ip
            font_size: 50
            size_hint_y: None
            height: 75
            text: '192.168.1.16'
            
        ConnectionButton:
            ip: ti_ip.text
            height: 75
            size_hint_y: None
            
        Label:
            
        BoxLayout:
            orientation: 'horizontal'
            
            Label:
                text: "MODE"
                
            BoxLayout:
                orientation: 'vertical'
                spacing: 10
                padding: 10
                
                CheckMode:
                    text: "RUN"
                    
                CheckMode:
                    text: "CAL"
                
                CheckMode:
                    text: "OTA"
                    
        SaveButton:
            height: 75
            size_hint_y: None
""")

class ConnectionLayout( TabDrawerItem ):
    pass
 
Builder.load_string("""
<ConnectionButton>:
    msg: 'connection'
    text: 'connected' if app.CONNECTED else self.msg
    on_release: root.connection()
""")
            
class ConnectionButton( Button ):
    
    def connection( self ):
        print "connection at", self.ip
        self.msg = 'connection ...'
        if APP.connect_to_bot( self, self.ip ):
            self.msg = 'connection ... ...'
        else :
            self.msg = 'connection'

Builder.load_string("""
<SaveButton>:
    disabled: not app.CONNECTED
    text: "save"
    on_release: self.send( "save", 1 )
""")

class SaveButton( Button, Sender ):
    pass

Builder.load_string("""
<CheckMode>:
    disabled: not app.CONNECTED
    on_state: self.send( "MODE", self.text )
    group: "MODE"
""")

class CheckMode( ToggleButton, Sender ):
    pass
    
# ================================================================
# ===                           Slider                         ===
# ================================================================

Builder.load_string("""
<MyS>:
    #ar: 1
    
    #canvas.before:
        #Color:
            #rgba: 1,0,0, self.ar * 0.5
        #Rectangle:
            #pos: self.pos
            #size: self.size
            
    size_hint: 1,None
    orientation: 'horizontal'
    spacing: 10
    height: 30
    
    Label:
        size_hint: None,1
        text: root.nom + ' = ' + str( Sp.value )
        
    Slider: 
        id: Sp
        range: root.rmin, root.rmax
        step: root.step
        value: root.value
        on_touch_up: root.send( root.nom, Sp.value ) if args[1].grab_current == self else None
""")

class MyS( BoxLayout, Receiver, Sender ):
    
    def __init__(self, values, **kwargs):
        self.nom = values[0]
        self.value, self.rmin, self.rmax, self.step = values[1]
        # make sure we aren't overriding any important functionality
        super(MyS, self).__init__(**kwargs)
        
    def on_msg_received( self, *args ):
        self.ids.Sp.value = float( self.msg_received )

# ================================================================
# ===                           Graph                          ===
# ================================================================

from kivy.garden.graph import Graph, MeshLinePlot, SmoothLinePlot, MeshStemPlot, ContourPlot
from kivy.uix.splitter import Splitter

class PidLayout( TabDrawerItem, Receiver):
    
    def __init__( self, tree, **kwargs ):
        # make sure we aren't overriding any important functionality
        super(PidLayout, self).__init__(**kwargs)
        
        self.text = tree[0]
                                    
        self.mys = ()
        for values in tree[1]:
            self.mys += ( MyS( values ), )
            
        self.bm = BoxLayout(
                            orientation = 'vertical',
                            spacing = 10,
                            padding = 10,
                            )
        self.bm.add_widget( self.build_bouton_layout( tree[2] ))
        self.bm.add_widget( self.build_graph( tree[2] ))
        self.add_widget(self.bm)
        
        #APP.add_receiver( self, self.text )
        self.key = self.text
        
    def on_menu( self, *args ):
        if self.menu:
            for s in self.mys:
                self.bm.add_widget( s )
        else:
            for s in self.mys:
                self.bm.remove_widget( s )
            
    def build_graph(self, tree):
        # example of a custom theme
        graph_theme = {
                'label_options': {
                    'color': (0.27,0.27,0.27,1),  # color of tick labels and titles
                    'bold': True
                    },
                'background_color': (0.97,0.97,0.95,1),  # back ground color of canvas
                'tick_color': (0.5,0.5,0.5,1),  # ticks and grid
                'border_color': (0.5,0.5,0.5,1) # border drawn around each graph
                }  
        
        self.graph = Graph(
                    xlabel='Seconde',
                    ylabel='Apples',
                    x_ticks_minor=0.5,
                    x_ticks_major=1,
                    y_ticks_minor=1,
                    y_ticks_major=10,
                    y_grid_label=True,
                    x_grid_label=True,
                    padding=5,
                    xlog=False,
                    ylog=False,
                    x_grid=True,
                    y_grid=True,
                    xmin=-3,
                    xmax=0,
                    ymin=-30,
                    ymax=30,
                    **graph_theme
                    )
        
        self.dpoints = []
        for nom,couleur in tree:
            self.dpoints.append(( nom, MeshLinePlot( color = couleur )))
        for dpoints in self.dpoints:
            self.graph.add_plot( dpoints[1] )
        
        #self.lpoints = [[0,0],[0,0],[0,0],[0,0],[0,0],[0,0]]
        self.lpoints = []
            
        return self.graph
        
        
    def update_points( self, points ):
        # points sous forme ( x, y1, y2,...)
        #print self.text + str(points)
        
        # passe le temps de ms en s
        #points[0] = points[0]/1000
        # Ajoute les derniers points a la liste temporaire
        self.lpoints.append( points )
        # trie la liste
        self.lpoints.sort( key = lambda x: x[0] )
        temps = self.lpoints[-1][0]
        # supprime les valeurs de plus de 3s
        while self.lpoints[0][0] < temps - 3000 :
            del self.lpoints[0]
        #print self.lpoints
        
        for i in range( 0, len( self.dpoints )):
            self.dpoints[i-1][1].points = ()
        # pour chaque point de la liste 
        for point in self.lpoints :
            # les range dans leurs listes de points sous forme (x,y)
            for i in range( 1, len( point )):
                self.dpoints[i-1][1].points.append((
                                                    ( point[0] - temps ) / 1000 ,
                                                    point[i] / 10 if i>1 else point[i]
                                                    ))
        
    def courbe_visibility( self, bout ):
        if bout.state is 'normal':
            self.graph.add_plot( self.dpoints[ bout.index ][1] )
            bout.background_color =  bout.couleur + ( 1,)
        else:
            self.graph.remove_plot( self.dpoints[ bout.index ][1] )
            bout.background_color =  bout.couleur + ( 0.5,)
            
    def build_bouton_layout( self, tree ):
        bl = BoxLayout(
                    size_hint= (1,None),
                    orientation= 'horizontal',
                    spacing= 10,
                    height= 30,
                    )
        i = 0
        for nom,couleur in tree :
            bl.add_widget( self.build_bouton( nom,couleur,i ))
            i += 1
        return bl
        
    def build_bouton( self, nom, couleur,i ):
        b = ToggleButton(
                        text = nom ,
                        background_normal = "",
                        background_down = ""
                        )
        b.nom = nom
        b.index = i
        b.couleur = couleur
        b.background_color =  b.couleur + ( 1,) 
        b.bind( on_release = self.courbe_visibility )
        return b
    
    def on_msg_received( self, *args ):
        #print self.msg_received
        msg = [ float(x) for x in self.msg_received.split() ]
        self.update_points( msg )

# ================================================================
# ===                            Pad                           ===
# ================================================================

from kivy.uix.relativelayout import RelativeLayout
from kivy.metrics import sp
            
class Pad(RelativeLayout):
    """
    Pad class 
    """
    text = 'control'
    
    def __init__(self, **kwargs):
        super(Pad, self).__init__(**kwargs)
        self.bind(
                pos = self.update_pad,
                size = self.update_pad
                )
        self.prev = (0,0)
        
    def update_pad(self, *args):
        '''
        Draw pad on canvas
        '''
        with self.canvas:
            self.canvas.clear()
            Color(1,1,1,1)
            unit = min( self.height , self.width )/13.
            cw = self.width/2
            ch = self.height/2
            Line( rectangle=(
                            cw - unit/2, 
                            ch - unit/2 - (unit * 5), 
                            unit,
                            unit * 11
                            ))
            Line( rectangle=(
                            cw - unit/2 - (unit * 5), 
                            ch - unit/2, 
                            unit * 11,
                            unit
                            ))
            for i in range(1,6):
                Line( rounded_rectangle=(
                                cw - unit/2 - (unit * i), 
                                ch - unit/2 - (unit * i), 
                                unit + unit * i * 2, 
                                unit + unit * i * 2, 
                                unit * i, 
                                unit * i, 
                                ))
            self.unit = unit
            self.cible = Rectangle(
                            size = (sp(64), sp(64)),
                            source = 'atlas://data/images/defaulttheme/slider_cursor{}'.format('')
                            )
        
        self.cible.pos = (
                        ( self.width - self.cible.size[0] ) /2,
                        ( self.height - self.cible.size[1]) /2
                        )
                
    def on_touch_down(self,touch):
        if self.collide_point( *touch.pos ):
            self.dep(touch)
            self.cible.pos = (
                            touch.pos[0] - self.cible.size[0]/2,
                            touch.pos[1] - self.cible.size[1]/2
                            )
        
    def on_touch_move( self, touch ):
            self.dep(touch)
            self.cible.pos = (
                            touch.pos[0] - self.cible.size[0]/2,
                            touch.pos[1] - self.cible.size[1]/2
                            )
        
    def on_touch_up( self, touch ):
        print "send STOP"
        self.prev = (0,0)
        self.cible.pos=(
                        ( self.width - self.cible.size[0] )/2,
                        ( self.height - self.cible.size[1] )/2
                        )
        
    def dep( self, touch ):
        mx = self.prop_touch( touch.x, self.width )
        my = self.prop_touch( touch.y, self.height )
        if (mx,my) != self.prev :
            print'send ',mx, my
        self.prev = (mx,my)
        
    def prop_touch( self, x, h ):
        m = ( x - h/2 ) / ( self.unit/2 )
        m = max( m , -10)
        m = min( m , 10)
        return int(m)

# ================================================================
# ===                            Log                           ===
# ================================================================

from kivy.garden.scrolllabel import ScrollLabel

Builder.load_string("""
<LogLayout>:
    text: 'Log'
    log: ''
    ScrollLabel:
        text: root.log
        halign: "left"
""")

class LogLayout(TabDrawerItem):
    
    def __init__(self, **kwargs):
        super(LogLayout, self).__init__(**kwargs)
        APP.add_log(self)
        
    def printlog(self, log):
        self.log += log +"\n"

# ================================================================
# ===                           Main                           ===
# ================================================================

from kivy.clock import Clock
from functools import partial

class BalancebotApp(App):
    
    # Parametres globlaux du robot
    CONNECTED = BooleanProperty(False)
    PAUSE = BooleanProperty(True)
    MODE = OptionProperty("None", options=["None", "RUN", "CAL", "OTA"])
    msg_sended = StringProperty( "" )
    
    def __init__(self, **kwargs):
        super(BalancebotApp, self).__init__(**kwargs)
        global APP
        APP = self
        self.sender = {}
        self.receiver = {}
    
    def build(self):
        return TabDrawer()
    
    def connect_to_bot( self, sender, host ):
        try:
            self.bot = Bot( host )
            reactor.listenUDP( 0, self.bot )
            self.msg_sended = "Bonjour"
            self.send()
            return 1
        except:
            print "echec de la connection"
            return 0
        
    def send( self, *args ):
        self.send_to_bot( self, self.msg_sended ) # no need for address
        self.clock = Clock.schedule_once( self.send, 0.5)
        
    def receive( self, msg ):
        print "-"+msg+"-"+self.msg_sended+"-"
        if msg == self.msg_sended :
            self.clock.cancel()
            
    def send_to_bot( self, sender, msg ):
        print "send : " + msg
        self.sender[msg] = sender
        self.bot.sendData(msg)
        
    def on_bot(self, msg):
        if not self.CONNECTED:
            self.CONNECTED = True
            self.send( "PID" )
        print "recu : " + msg
        commande = msg.split()[0]
        #print msg
        #print msg.find(" ")
        #print msg.strip()[ msg.find(" ") + 1 : ]
        msg = msg.strip()[ msg.find(" ") + 1 : ]
            
        if commande == 'ar':
            if msg in self.sender :
                self.sender[msg].receive(msg)
        
        elif commande in self.receiver :
            self.receiver[ commande ].msg_received = msg
            
        elif commande == 'PID':
            self.printlog(msg)
            while msg:
                commande = msg[0]
                if commande in self.receiver:
                    self.receiver[ commande ].msg_received = msg[1] 
                if len( msg ) == 2:
                    break
                msg = msg[2:]
            
        else :
            self.printlog( 'inc : ' + commande + str(msg) )
        
    def add_receiver( self, receiver ):
        self.receiver[ receiver.key ] = receiver
        
    def add_log( self, log ):
        self.log = log
        
    def printlog(self,msg):
        try:
            self.log.printlog(msg)
        except:
            print msg
            
if __name__ == '__main__':
    BalancebotApp().run()
   
