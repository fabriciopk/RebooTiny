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

export class PlugRelay_app extends Component {
  constructor(props){
    super(props);
    
    this.state = {
      plugOff: 0,
      loading: true,
      errorMessage: '',
      version: '',
    };
    
    this.on = "ON";
    this.off = "OFF";
    
    this.plug_getState();
  }
  
  plug_getState(){
    fetch('http://192.168.2.123/rest/get').then((res) => res.json()).then((res) => {
     this.setState({plugOff: res.enabled, version: res.version, loading: false, errorMessage: ''})
    }).catch((err) => {this.setState({errorMessage: 'Error communicating to PlugRelay', loading: false}); console.log(err);});
  }
  
  plug_toggle(){
    if (this.state.loading)
      return;
    
    this.setState({loading: true});
      fetch('http://192.168.2.123/rest/post/toggle').then((res) => res.json()).then((res) => {
        if (res.success){
          this.setState({errorMessage: ''});
          this.plug_getState();
        }
      }).catch((err) =>{
        this.setState({loading: false, errorMessage: 'Error communicating to PlugRelay'});
        console.log(err);
      });
  }
  
  render() {    
    return (
      <View style={{flex:1}}>           

        <View style={{flex:8}}>
          <TouchableOpacity
            style={{flex:1}}
            onPress={() => {this.plug_toggle()}}
            activeOpacity={75 / 100}>
            <View
              style={{
                flex: 1,
                justifyContent: "center",
                alignItems: "center",
                backgroundColor: (this.state.plugOff == 1)? "rgb(210,210,210)": "rgba(248,231,28,0.75)",
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
                  Plug: {(this.state.errorMessage == '')? ((this.state.plugOff==0)? this.on:this.off) : this.state.errorMessage}
                </Text>
              }
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
            Version: {this.state.version}
          </Text>
          
          <TouchableOpacity
            style = {{flex:1}}
            onPress={() => {
              this.setState({loading:true});
              this.plug_getState();
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
