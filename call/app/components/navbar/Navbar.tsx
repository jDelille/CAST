import React from "react";
import Button from "../reusable/button/Button"

  type YoProps = {
    isAuthPage?: boolean;
  };

  const Yo: React.FC<YoProps> = ({isAuthPage}) => {

    const handleClick = () => {
      console.log("Clicked...");
    }

    return (
      <nav className="navbar">
        <div className="logo">
          <img src="" alt="" />
        </div>
        <Button text={isAuthPage ? "Login" : "Signup"} onClick={handleClick} />
      </nav>
    );
  }

  export default Yo;

