/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 * @flow
 */

import React, { Component } from 'react';
import {
  Text,
  View,
  TouchableOpacity,
  ActivityIndicator,
  Alert,
  Image,
} from 'react-native'

export class RebooTinny_app extends Component {
  constructor(props){
    super(props);
    
    this.state = {
      rebooEnabled: 0,
      rebooReboots: '',
      rebooVersion: '',
      loading: true,
      errorMessage: '',
    };
    
    this.reboo_getState();
  }
  
  reboo_getState(){
    fetch('http://192.168.1.10/rest/get').then((res) => res.json()).then((res) => {
     this.setState({rebooEnabled: res.enabled, rebooReboots: res.reboots, rebooVersion: res.version, loading: false, errorMessage: ''})
    }).catch((err) => {this.setState({errorMessage: 'Error accessing RebooTinny', loading: false}); console.log(err);});
  }
  
  reboo_reboot(){
    Alert.alert('WOW!', 
                'Take it easy, dear friend! Are you sure?', 
    [
      {text: "Cancelar"},
      {text: "Rebootar", onPress: () => {
        this.setState({loading: true});
        fetch('http://192.168.1.10/rest/post/reboot').then((res) => res.json()).then((res) => {
          this.setState({loading: false});
          if (res.ok)
            Alert.alert('Wait while it reboots!');
          else
            this.setState({errorMessage: 'Could not Reboot, try again later'});
        }).catch((err) => {
          this.setState({loading: false, errorMessage: 'Error accessing RebooTinny'});
          console.log(err);
        });
      }},      
    ]);
  }
  
  reboo_enable(){
    if (this.state.loading)
      return;
    
    this.setState({loading: true});
    fetch('http://192.168.1.10/rest/post/enable').then((res) => res.json()).then((res) => {
      if (res.ok){
        this.setState({errorMessage: ''});
        this.reboo_getState();
      }
    }).catch(() =>{
          this.setState({loading: false, errorMessage: 'Error sending command to RebooTinny'});
        });
  }
  
  render() {    
    return (
      <View style={{flex:1}}>           

        <View style={{flex:4}}>
          <TouchableOpacity
            style={{flex:1}}
            onPress={() => {this.reboo_enable()}}
            activeOpacity={75 / 100}>
            <View
              style={{
                flex: 1,
                justifyContent: "center",
                alignItems: "center",
                backgroundColor: (this.state.rebooEnabled == 1)? "rgba(89,198,113,1)": "rgba(218,59,59,1)",
              }}>
              {(this.state.loading)?
                <ActivityIndicator
                  style={{
                    alignItems: 'center',
                    justifyContent: 'center',
                  }}
                  animating={true}
                  size={"small"}
                  color={'black'}
                />
               : 
                <Text style={{textAlign: 'center'}}>
                  Enabled: {(this.state.errorMessage == '')? this.state.rebooEnabled : this.state.errorMessage}
                </Text>
              }
            </View>
          </TouchableOpacity> 
        </View>
        
        <View style={{flex:4}}>
          <TouchableOpacity
            style={{flex:1}}
            onPress={() => {this.reboo_reboot()}}
            activeOpacity={75 / 100}>
            <View
              style={{
                flex: 1,
                justifyContent: "center",
                alignItems: "center",
                backgroundColor: "rgba(226,225,74,1)",
              }}>
              <Text style={{textAlign:'center'}}>
                Reboots: {this.state.rebooReboots}
              </Text>
            </View>
          </TouchableOpacity>
        </View>
        
        <View
          style={{
            flex: 1,
            flexDirection: 'row',
            justifyContent: "center",
            alignItems: "center",
            backgroundColor: "rgba(112,154,203,1)",
          }}>
          <Text style={{textAlign:'center', flex:3}}>
            Version: {this.state.rebooVersion}
          </Text>
          
          <TouchableOpacity
            style = {{flex:1}}
            onPress={() => {
              this.setState({loading:true});
              this.reboo_getState();
            }}
            activeOpacity={36 / 100}>
            <Image 
              style={{
                flex:1,
                width: 28,
                height: 28,
              }}
              resizeMode={"contain"}
              source={require('./static/img/refresh.png')}
            />
          </TouchableOpacity>
        </View>
        
      </View>  
    );
  }
}
